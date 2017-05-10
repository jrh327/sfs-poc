#include "sfs.h"
#include "util.h"
#include "allocation.h"
#include "files.h"

struct sfs_filesystem* initialize_new_filesystem(int fd, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster) {
    return (initialize_filesystem_partition(fd, 0, fat_size, bytes_per_sector,
            sectors_per_cluster));
}

struct sfs_filesystem* initialize_filesystem_partition(int fd,
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
    sfs->fd = fd;
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

    write_to_file(fd, &arr_boot_sector, BOOT_SECTOR_SIZE);

    /* initialize the file allocation table */
    uint8_t* arr_fat_sector = calloc(1, sfs->bytes_per_sector);
    size_t num_fat_sectors = (sfs->entries_per_fat * FAT_ENTRY_SIZE)
            / sfs->bytes_per_sector;
    for (size_t i = 0; i < num_fat_sectors; i++) {
        write_to_file(fd, arr_fat_sector, sfs->bytes_per_sector);
    }
    free(arr_fat_sector);

    /* mark the first cluster as taken by the root directory */
    jump_to_fat(sfs, 0);
    write_uint16(fd, END_CLUSTER_CHAIN);
    write_uint16(fd, END_CLUSTER_CHAIN);

    sfs->first_available_fat_entry = find_next_avail_fat_entry(sfs,
            (const struct fat_entry){ 0 });

    /* initialize a cluster for the root directory */
    /* generated automatically by the jump */
    jump_to_cluster(sfs, (const struct fat_entry){ 0 });

    return (sfs);
}

struct sfs_filesystem* load_filesystem(int fd) {
    /* read the first three bytes to check if this is an SFS filesystem */
    if (read_uint8(fd) != 'S' || read_uint8(fd) != 'F'
            || read_uint8(fd) != 'S') {
        printf("Given file does not represent an SFS filesystem.\n");
        return (NULL);
    }

    /* skip to the actual data */
    seek_in_file(fd, 8, SEEK_SET);

    struct sfs_filesystem* sfs = malloc(sizeof(struct sfs_filesystem));
    sfs->fd = fd;
    sfs->partition_offset = read_uint64(fd);
    sfs->entries_per_fat = read_uint16(fd);
    sfs->bytes_per_sector = read_uint16(fd);
    sfs->sectors_per_cluster = read_uint8(fd);
    sfs->first_available_fat_entry = find_next_avail_fat_entry(sfs,
            (const struct fat_entry){ 0 });

    return (sfs);
}

int close_filesystem(struct sfs_filesystem* sfs) {
    int error = close_file(sfs->fd);

    if (!error) {
        free_sfs(sfs);
    }

    return (error);
}

struct directory_entry* get_root_directory(const struct sfs_filesystem* sfs) {
    return (get_root_directory_entry(sfs));
}

int list_directory(const struct sfs_filesystem* sfs,
        struct directory_entry* dir) {
    return (get_directory_entries(sfs, dir));
}

struct directory_entry* create_file(const struct sfs_filesystem* sfs,
        struct directory_entry* parent, const char* filename,
        const uint8_t* data, const uint64_t file_length) {
    struct directory_entry* entry = create_directory_entry(parent, filename,
            file_length);

    struct fat_list* clusters = allocate_file(sfs, file_length);
    entry->table_number = clusters->entry->fat_number;
    entry->first_cluster = clusters->entry->cluster_number;

    write_directory_entry(sfs, entry);

    write_file_clusters(sfs, clusters, data, file_length);

    free_fat_list(clusters);

    return (entry);
}

uint8_t* read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    struct fat_list* clusters = get_file_clusters(sfs, file);

    uint8_t* data = read_file_clusters(sfs, clusters, file->file_length);

    free_fat_list(clusters);

    return (data);
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
    int old_filename_entries = file->filename_entries;
    change_file_name(sfs, file, filename);

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
