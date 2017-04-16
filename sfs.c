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
    return (NULL);
}

struct directory_entry* list_directory(const struct directory_entry* dir) {
    return (NULL);
}

struct directory_entry* create_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* parent, const uint8_t* data) {
    return (NULL);
}

uint8_t* read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    return (NULL);
}

int update_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, const uint8_t* data, int offset) {
    return (0);
}

int delete_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    return (0);
}

int hard_delete_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    return (0);
}

int rename_file(const struct sfs_filesystem* sfs, struct directory_entry* file,
        const char* filename) {
    return (0);
}

int move_file(const struct sfs_filesystem* sfs, struct directory_entry* file,
        const struct directory_entry* new_parent) {
    return (0);
}
