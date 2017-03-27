#include "sfs.h"
#include "util.h"

#define FAT_SIZE_SMALL 2048
#define FAT_SIZE_MEDIUM 4096
#define FAT_SIZE_LARGE 8192

#define BOOT_SECTOR_SIZE 512

struct boot_sector* initialize_new_filesystem() {
    struct boot_sector* sfs = malloc(sizeof(struct boot_sector));
    char* filename = NULL;
    size_t len = 0;

    printf("Please enter the name of the filesystem: ");
    while ((len = getline(&filename, &len, stdin)) <= 0) {
        printf("Please enter the name of the filesystem: ");
    }

    /*
     * open the file for read/write in binary mode
     * creating a new filesystem so truncate the file if it exists already
     */
    FILE* fp = fopen(filename, "wb+");
    if (fp == NULL) {
        printf("error creating or opening the file\n");
        return NULL;
    }

    sfs->fp = fp;

    char choice;
    printf("Is this a partition? [y/n]: ");
    scanf("%s", &choice);
    if (choice == 'y' || choice == 'Y') {
        uint64_t offset;
        printf("Please enter the parition's offset: ");
        scanf("%llu", &offset);
        sfs->partition_offset = offset;
    } else {
        sfs->partition_offset = 0;
    }



    printf("Please enter the size of the filesystem's allocation tables:\n");
    printf("1) Small  (2K entries per table)\n");
    printf("2) Medium (4k entries per table)\n");
    printf("3) Large  (8K entries per table)\n");
    printf("Choice (default Medium): ");
    scanf("%s", &choice);
    switch (choice) {
    case 1:
        sfs->entries_per_fat = FAT_SIZE_SMALL;
        break;
    case 2:
        sfs->entries_per_fat = FAT_SIZE_MEDIUM;
        break;
    case 3:
        sfs->entries_per_fat = FAT_SIZE_LARGE;
        break;
    default:
        sfs->entries_per_fat = FAT_SIZE_MEDIUM;
        break;
    }
    printf("Set number of entries to file allocation table to %d.\n\n",
            sfs->entries_per_fat);

    uint16_t bytes_per_sector;
    printf("Please enter the number of bytes per sector.\n");
    printf("Must be a power of 2. Minimum 512, maximum 32768.\n");
    printf("(default 512): ");
    scanf("%hd", &bytes_per_sector);

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
    printf("Set number of bytes per sector to %d.\n\n", sfs->bytes_per_sector);

    uint8_t sectors_per_cluster;
    printf("Please enter the number of sectors per cluster.\n");
    printf("Must be a power of 2, and maximum cluster size is 32K: ");
    scanf("%s", &sectors_per_cluster);

    if (sectors_per_cluster * bytes_per_sector > 0x8000) {
        sectors_per_cluster = 0x8000 / bytes_per_sector;
    } else {
        uint8_t bitpos = 0x80;
        while (!(sectors_per_cluster & bitpos)) {
            bitpos = bitpos >> 1;
        }
    }

    sfs->sectors_per_cluster = sectors_per_cluster;
    printf("Set number of sectors per cluster to %d.\n\n",
            sfs->sectors_per_cluster);

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
        write_uint16(fp, 0);
    }

    /* initialize a cluster for the root directory */
    uint16_t cluster_size = sfs->sectors_per_cluster * sfs->bytes_per_sector;
    for (size_t i = 0; i < cluster_size; i++) {
        write_uint8(fp, 0);
    }

    return sfs;
}

struct boot_sector* load_filesystem() {
    char* filename = NULL;
    size_t len = 0;

    printf("Please enter the name of the filesystem: ");
    while ((len = getline(&filename, &len, stdin)) <= 0) {
        printf("Please enter the name of the filesystem: ");
    }

    FILE* fp = fopen(filename, "rb+");
    if (fp == NULL) {
        printf("error creating or opening the file\n");
        return NULL;
    }

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

uint16_t get_fat_entry(struct boot_sector* sfs, uint16_t fat_number,
        uint16_t entry_number) {
    move_to_fat(sfs, fat_number);

    FILE* fp = sfs->fp;
    fseek(fp, entry_number * 2, SEEK_CUR);

    uint16_t entry = read_uint16(fp);
    return entry;
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
