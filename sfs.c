#include "sfs.h"
#include "util.h"

struct boot_sector* initialize_new_filesystem(FILE* fp, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster) {
    return initialize_filesystem_partition(fp, 0, fat_size,
            bytes_per_sector, sectors_per_cluster);
}

struct boot_sector* initialize_filesystem_partition(FILE* fp,
        uint64_t partition_offset, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster) {
    struct boot_sector* sfs = malloc(sizeof(struct boot_sector));

    sfs->fp = fp;
    sfs->partition_offset = partition_offset;

    if (fat_size != FAT_SIZE_SMALL
            && fat_size != FAT_SIZE_MEDIUM
            && fat_size != FAT_SIZE_LARGE) {
        fat_size = FAT_SIZE_MEDIUM;
    }
    sfs->entries_per_fat = fat_size;

    if (bytes_per_sector < 512) {
        bytes_per_sector = 512;
    } else {
        uint16_t bitpos = 0x8000;
        while (!(bytes_per_sector & bitpos)) {
            bitpos = bitpos >> 1;
        }
        bytes_per_sector = bitpos;
    }
    sfs->bytes_per_sector = bytes_per_sector;

    if (sectors_per_cluster * bytes_per_sector > 0x8000) {
        sectors_per_cluster = 0x8000 / bytes_per_sector;
    } else {
        uint8_t bitpos = 0x80;
        while (!(sectors_per_cluster & bitpos)) {
            bitpos = bitpos >> 1;
        }
        sectors_per_cluster = bitpos;
    }
    sfs->sectors_per_cluster = sectors_per_cluster;

    write_uint8(fp, 'S');
    write_uint8(fp, 'F');
    write_uint8(fp, 'S');
    write_uint8(fp, ' ');
    write_uint8(fp, 'v');
    write_uint8(fp, '1');
    write_uint8(fp, '.');
    write_uint8(fp, '0');
    write_uint64(fp, sfs->partition_offset);
    write_uint16(fp, sfs->entries_per_fat);
    write_uint16(fp, sfs->bytes_per_sector);
    write_uint8(fp, sfs->sectors_per_cluster);

    /* already wrote 21 bytes to the boot sector, zero out the rest */
    for (size_t i = 21; i < BOOT_SECTOR_SIZE; i++) {
        write_uint8(fp, 0);
    }

    /* initialize the file allocation table */
    uint16_t fat_entries = sfs->entries_per_fat;
    for (size_t i = 0; i < fat_entries; i++) {
        /* write 4 bytes per entry: 2 for fat number, 2 for next cluster */
        write_uint32(fp, 0);
    }

    /* initialize a cluster for the root directory */
    uint16_t cluster_size = sfs->sectors_per_cluster * sfs->bytes_per_sector;
    for (size_t i = 0; i < cluster_size; i++) {
        write_uint8(fp, 0);
    }

    return sfs;
}

struct boot_sector* load_filesystem(FILE* fp) {
    /* read the first three bytes to check if this is an SFS filesystem */
    if (read_uint8(fp) != 'S'
          || read_uint8(fp) != 'F'
          || read_uint8(fp) != 'S') {
        printf("Given file does not represent an SFS filesystem.\n");
        return NULL;
    }

    /* skip to the actual data */
    fseek(fp, 8, SEEK_SET);

    struct boot_sector* sfs = malloc(sizeof(struct boot_sector));

    sfs->fp = fp;
    sfs->partition_offset = read_uint64(fp);
    sfs->entries_per_fat = read_uint16(fp);
    sfs->bytes_per_sector = read_uint16(fp);
    sfs->sectors_per_cluster = read_uint8(fp);

    return sfs;
}

void close_filesystem(FILE* fp) {
    fclose(fp);
}

void move_to_fat(struct boot_sector* sfs, uint16_t fat_number) {
    FILE* fp = sfs->fp;

    /* move to the beginning of the first FAT */
    fseek(fp, BOOT_SECTOR_SIZE, SEEK_SET);

    if (fat_number == 0) {
        return;
    }

    /* 
     * loop until the correct FAT is found. FAT number is zero-based,
     * if fat_number == 0, already at correct FAT and this is skipped
     */
    uint16_t fat_size = sfs->entries_per_fat * 2; /* two bytes per entry */
    uint32_t data_block_size = sfs->bytes_per_sector
            * sfs->sectors_per_cluster * sfs->entries_per_fat;
    while (fat_number) {
        /* move to the end of the current FAT */
        fseek(fp, fat_size, SEEK_CUR);

        /* move to the end of the current data block */
        fseek(fp, data_block_size, SEEK_CUR);

        fat_number--;
    }
}

struct fat_entry get_fat_entry(struct boot_sector* sfs,
        struct fat_entry entry) {
    move_to_fat(sfs, entry.fat_number);

    FILE* fp = sfs->fp;
    fseek(fp, entry.cluster_number * 4, SEEK_CUR);

    uint16_t entry_fat_number = read_uint16(fp);
    uint16_t entry_cluster_number = read_uint16(fp);

    struct fat_entry new_entry = {
            .fat_number = entry_fat_number,
            .cluster_number = entry_cluster_number
    };
    return new_entry;
}

void move_to_cluster(struct boot_sector* sfs, uint16_t data_block_number,
        uint16_t cluster_number) {
    /* move to the FAT before the cluster's data block */
    move_to_fat(sfs, data_block_number);

    FILE* fp = sfs->fp;

    /* move to the end of the current FAT */
    fseek(fp, sfs->entries_per_fat * 2, SEEK_CUR);

    if (cluster_number == 0) {
        return;
    }

    /* 
     * loop until the correct cluster is found. cluster number is zero-based,
     * if cluster_number == 0, already at correct cluster and this is skipped
     */
    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    while (cluster_number) {
        /* move to the end of the current cluster */
        fseek(fp, cluster_size, SEEK_CUR);

        cluster_number--;
    }
}

uint8_t* get_cluster(struct boot_sector* sfs, uint16_t data_block_number,
        uint16_t cluster_number) {
    move_to_cluster(sfs, data_block_number, cluster_number);

    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint8_t* cluster = malloc(cluster_size);

    fread(cluster, cluster_size, 1, sfs->fp);
    return cluster;
}
