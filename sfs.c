#include "sfs.h"
#include "util.h"

struct boot_sector initialize_new_filesystem(FILE* fp, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster) {
    return initialize_filesystem_partition(fp, 0, fat_size,
            bytes_per_sector, sectors_per_cluster);
}

struct boot_sector initialize_filesystem_partition(FILE* fp,
        uint64_t partition_offset, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster) {
    if (fat_size != FAT_SIZE_SMALL
            && fat_size != FAT_SIZE_MEDIUM
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

    struct boot_sector sfs = {
        .fp = fp,
        .partition_offset = partition_offset,
        .entries_per_fat = fat_size,
        .bytes_per_sector = bytes_per_sector,
        .sectors_per_cluster = sectors_per_cluster,
    };

    uint8_t arr_boot_sector[BOOT_SECTOR_SIZE] = {
        'S', 'F', 'S', ' ', 'v', '1', '.', '0'
    };

    put_uint64(arr_boot_sector, sfs.partition_offset, 8);
    put_uint16(arr_boot_sector, sfs.entries_per_fat, 16);
    put_uint16(arr_boot_sector, sfs.bytes_per_sector, 18);
    arr_boot_sector[20] = sfs.sectors_per_cluster;

    fwrite(&arr_boot_sector, sizeof(arr_boot_sector), 1, fp);


    /* initialize the file allocation table */
    uint8_t arr_fat_sector[512];
    size_t num_fat_sectors = (sfs.entries_per_fat * sizeof(struct fat_entry)) / 512;
    for (size_t i = 0; i < num_fat_sectors; i++) {
        fwrite(&arr_fat_sector, sizeof(arr_fat_sector), 1, fp);
    }

    /* initialize a cluster for the root directory */
    uint8_t arr_cluster_sector[sfs.bytes_per_sector];
    size_t num_cluster_sectors = sfs.sectors_per_cluster;
    for (size_t i = 0; i < num_cluster_sectors; i++) {
        fwrite(&arr_cluster_sector, sizeof(arr_cluster_sector), 1, fp);
    }

    return sfs;
}

struct boot_sector load_filesystem(FILE* fp) {
    /* read the first three bytes to check if this is an SFS filesystem */
    if (read_uint8(fp) != 'S'
          || read_uint8(fp) != 'F'
          || read_uint8(fp) != 'S') {
        printf("Given file does not represent an SFS filesystem.\n");
        struct boot_sector empty = { 0 };
        return empty;
    }

    /* skip to the actual data */
    fseek(fp, 8, SEEK_SET);

    struct boot_sector sfs = {
        .fp = fp,
        .partition_offset = read_uint64(fp),
        .entries_per_fat = read_uint16(fp),
        .bytes_per_sector = read_uint16(fp),
        .sectors_per_cluster = read_uint8(fp)
    };

    return sfs;
}

void close_filesystem(FILE* fp) {
    fclose(fp);
}

void move_to_fat(struct boot_sector sfs, uint16_t fat_number) {
    FILE* fp = sfs.fp;

    /* move to the beginning of the first FAT */
    fseek(fp, BOOT_SECTOR_SIZE, SEEK_SET);

    if (fat_number == 0) {
        return;
    }

    /* 
     * loop until the correct FAT is found. FAT number is zero-based,
     * if fat_number == 0, already at correct FAT and this is skipped
     */
    uint16_t fat_size = sfs.entries_per_fat * sizeof(struct fat_entry);
    uint32_t data_block_size = sfs.bytes_per_sector * sfs.sectors_per_cluster;
    while (fat_number) {
        /* move to the end of the current FAT */
        fseek(fp, fat_size, SEEK_CUR);

        /* move to the end of the current data block */
        fseek(fp, data_block_size, SEEK_CUR);

        fat_number--;
    }
}

struct fat_entry get_fat_entry(const struct boot_sector sfs,
        struct fat_entry entry) {
    move_to_fat(sfs, entry.fat_number);

    FILE* fp = sfs.fp;
    fseek(fp, entry.cluster_number * sizeof(struct fat_entry), SEEK_CUR);

    uint16_t entry_fat_number = read_uint16(fp);
    uint16_t entry_cluster_number = read_uint16(fp);

    struct fat_entry new_entry = {
            .fat_number = entry_fat_number,
            .cluster_number = entry_cluster_number
    };
    return new_entry;
}

void move_to_cluster(const struct boot_sector sfs,
        const struct fat_entry entry) {
    /* move to the FAT before the cluster's data block */
    move_to_fat(sfs, entry.cluster_number);

    FILE* fp = sfs.fp;

    /* move to the end of the current FAT */
    fseek(fp, sfs.entries_per_fat * sizeof(struct fat_entry), SEEK_CUR);

    if (entry.cluster_number == 0) {
        return;
    }

    /* 
     * loop until the correct cluster is found. cluster number is zero-based,
     * if cluster_number == 0, already at correct cluster and this is skipped
     */
    uint32_t cluster_size = sfs.bytes_per_sector * sfs.sectors_per_cluster;
    uint16_t cluster_number = entry.cluster_number;
    while (cluster_number) {
        /* move to the end of the current cluster */
        fseek(fp, cluster_size, SEEK_CUR);

        cluster_number--;
    }
}

uint8_t* get_cluster(const struct boot_sector sfs,
        const struct fat_entry entry) {
    move_to_cluster(sfs, entry);

    uint32_t cluster_size = sfs.bytes_per_sector * sfs.sectors_per_cluster;
    uint8_t* cluster = malloc(cluster_size);

    fread(cluster, cluster_size, 1, sfs.fp);
    return cluster;
}
