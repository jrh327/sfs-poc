#include "sfs.h"
#include "util.h"
#include "files.h"

struct sfs_filesystem* initialize_new_filesystem(FILE* fp, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster) {
    return (initialize_filesystem_partition(fp, 0, fat_size, bytes_per_sector,
            sectors_per_cluster));
}

struct sfs_filesystem* initialize_filesystem_partition(FILE* fp,
        uint64_t partition_offset, uint16_t fat_size, uint16_t bytes_per_sector,
        uint8_t sectors_per_cluster) {
    if (fat_size != FAT_SIZE_SMALL && fat_size != FAT_SIZE_MEDIUM
            && fat_size != FAT_SIZE_LARGE) {
        fat_size = FAT_SIZE_MEDIUM;
    }

    if (bytes_per_sector < 512) {
        bytes_per_sector = 512;
    } else {
        uint16_t bitpos = 0x8000;
        while (!(bytes_per_sector & bitpos)) {
            bitpos = bitpos >> 1;
        }
        bytes_per_sector = bitpos;
    }

    if (sectors_per_cluster * bytes_per_sector > 0x8000) {
        sectors_per_cluster = 0x8000 / bytes_per_sector;
    } else {
        uint8_t bitpos = 0x80;
        while (!(sectors_per_cluster & bitpos)) {
            bitpos = bitpos >> 1;
        }
        sectors_per_cluster = bitpos;
    }

    struct sfs_filesystem* sfs = malloc(sizeof(struct sfs_filesystem));
    sfs->fp = fp;
    sfs->partition_offset = partition_offset;
    sfs->entries_per_fat = fat_size;
    sfs->bytes_per_sector = bytes_per_sector;
    sfs->sectors_per_cluster = sectors_per_cluster;

    uint8_t arr_boot_sector[BOOT_SECTOR_SIZE] = {
            'S', 'F', 'S', ' ', 'v', '1', '.', '0'
    };

    put_uint64(arr_boot_sector, sfs->partition_offset, 8);
    put_uint16(arr_boot_sector, sfs->entries_per_fat, 16);
    put_uint16(arr_boot_sector, sfs->bytes_per_sector, 18);
    arr_boot_sector[20] = sfs->sectors_per_cluster;

    fwrite(&arr_boot_sector, sizeof(arr_boot_sector), 1, fp);

    /* initialize the file allocation table */
    uint8_t arr_fat_sector[512];
    size_t num_fat_sectors = (sfs->entries_per_fat * sizeof(struct fat_entry))
            / 512;
    for (size_t i = 0; i < num_fat_sectors; i++) {
        fwrite(&arr_fat_sector, sizeof(arr_fat_sector), 1, fp);
    }

    /* initialize a cluster for the root directory */
    uint8_t arr_cluster_sector[sfs->bytes_per_sector];
    size_t num_cluster_sectors = sfs->sectors_per_cluster;
    for (size_t i = 0; i < num_cluster_sectors; i++) {
        fwrite(&arr_cluster_sector, sizeof(arr_cluster_sector), 1, fp);
    }

    return (sfs);
}

struct sfs_filesystem* load_filesystem(FILE* fp) {
    /* read the first three bytes to check if this is an SFS filesystem */
    if (read_uint8(fp) != 'S' || read_uint8(fp) != 'F'
            || read_uint8(fp) != 'S') {
        printf("Given file does not represent an SFS filesystem.\n");
        return (NULL);
    }

    /* skip to the actual data */
    fseek(fp, 8, SEEK_SET);

    struct sfs_filesystem* sfs = malloc(sizeof(struct sfs_filesystem));
    sfs->fp = fp;
    sfs->partition_offset = read_uint64(fp);
    sfs->entries_per_fat = read_uint16(fp);
    sfs->bytes_per_sector = read_uint16(fp);
    sfs->sectors_per_cluster = read_uint8(fp);

    return (sfs);
}

int close_filesystem(struct sfs_filesystem* sfs) {
    int ret = fclose(sfs->fp);

    if (!ret) {
        free(sfs);
    }

    return (ret);
}

struct directory_entry* get_root_directory(const struct sfs_filesystem* sfs) {
    return (get_root_directory_entry(sfs));
}

int list_directory(const struct sfs_filesystem* sfs,
        struct directory_entry* dir) {
    return (get_directory_entries(sfs, dir));
}

/*
 * Return the number of bytes representing the passed UTF-8 filename.
 * If the \0 terminator is not found within 1020 bytes or if the filename
 * contains more than 255 UTF-8 characters, return 0.
 */
int get_filename_length(const char* filename) {
    const int max_bytes = 1020;
    const int max_chars = 255;
    int bytes = 0;
    int chars = 0;
    int codepoint_length = 0;

    while (bytes <= max_bytes && chars <= max_chars) {
        char cur = *(filename + bytes);
        if (!cur) {
            break;
        }

        /*
         * check how many bytes the current character spans.
         *
         * 1 byte for ascii characters
         * 2-byte code points' first byte like 0b110xxxxx
         * 3-byte code points' first byte like 0b1110xxxx
         * 4-byte code points' first byte like 0b11110xxx
         *
         * anything else should be like 0b10xxxxxx, but don't
         * waste time checking for malformed unicode; let the
         * client deal with that; only constraining on byte-
         * and character-length here, and messed up bytes that
         * keep resetting the codepoint length counter will be
         * stopped by the total bytes counter.
         */
        if ((int)cur < 0x80) {
            codepoint_length = 1;
        } else if ((int)cur > 0xf0) {
            codepoint_length = 4;
        } else if ((int)cur > 0xe0) {
            codepoint_length = 3;
        } else if ((int)cur > 0xc0) {
            codepoint_length = 2;
        }

        codepoint_length--;
        if (codepoint_length == 0) {
            chars++;
        }

        bytes++;
    }

    if (bytes <= max_bytes && chars <= max_chars) {
        return (bytes);
    }

    return (0);
}

struct directory_entry* create_file(const struct sfs_filesystem* sfs,
        struct directory_entry* parent, const char* filename,
        const uint8_t* data, const uint64_t file_length) {
    struct directory_entry* entry = malloc(sizeof(struct directory_entry));
    int filename_length = get_filename_length(filename);
    if (filename_length > 0) {
        entry->filename = malloc((sizeof(char) * filename_length) + 1);
        memcpy(entry->filename, filename, filename_length);
    } else {
        filename_length = 1020;
        entry->filename = malloc((sizeof(char) * filename_length) + 1);
        memcpy(entry->filename, filename, filename_length);
    }
    entry->filename[filename_length] = 0;
    entry->filename_entries = (filename_length - 11) / (DIR_ENTRY_SIZE - 1);

    entry->reserved = 0;
    entry->parent = parent;
    entry->file_length = file_length;
    // need to decide how to set attributes
    entry->attributes = 0;
    // created and modified date = now
    // allocate a chain of clusters in the allocation table
    // table_number = allocation table number of first cluster
    // first_cluster = number of first cluster in chain
    // write entry to parent directory
    // write date to clusters

    return (entry);
}

uint8_t* read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    // data = malloc(sizeof(char) * file->file_length);
    // for cluster in clusters: read into data
    return (NULL);
}

int update_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, const uint8_t* data, int offset) {
    // really not sure if I want to have all data after first change passed in
    // or pass individual diffs for each change to the file
    // or possibly just have entire new data passed in and overwrite old data
    return (0);
}

int delete_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file) {
    file->reserved &= 0xc0;
    for (int i = 0; i < file->filename_entries; i++) {
        // get next filename entry
        // filename_entry->reserved &= 0xc0;
    }
    return (0);
}

int undelete_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file) {
    file->reserved &= 0x3f;
    for (int i = 0; i < file->filename_entries; i++) {
        // get next filename entry
        // filename_entry->reserved &= 0x3f;
    }
    return (0);
}

int hard_delete_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    // follow cluster chain, zero out all bytes
    // zero out all bytes of directory entry
    // move up any following directory entries
    return (0);
}

int rename_file(const struct sfs_filesystem* sfs, struct directory_entry* file,
        const char* filename) {
    int filename_length = get_filename_length(filename);
    if (filename_length > 0) {
        file->filename = malloc((sizeof(char) * filename_length) + 1);
        memcpy(file->filename, filename, filename_length);
    } else {
        filename_length = 1020;
        file->filename = malloc((sizeof(char) * filename_length) + 1);
        memcpy(file->filename, filename, filename_length);
    }
    file->filename[filename_length] = 0;

    int old_filename_entries = file->filename_entries;
    file->filename_entries = (filename_length - 11) / (DIR_ENTRY_SIZE - 1);

    if (file->filename_entries > old_filename_entries) {
        // push back any following directory entries
        // write new filename entries
    } else {
        // write new filename entries
        // move up any following directory entries
    }

    return (0);
}

int move_file(const struct sfs_filesystem* sfs, struct directory_entry* file,
        const struct directory_entry* new_parent) {
    // write directory entry to new parent
    // zero out all bytes of entry in old parent
    // move up any following directory entries in old parent
    return (0);
}
