#include "files.h"

struct directory_entry read_directory_entry(const struct sfs_filesystem sfs,
        struct directory_entry* parent) {
    uint8_t entry[32] = { 0 };
    fread(&entry, sizeof(entry), 1, sfs.fp);

    uint8_t created_month = (entry[2] & 0xf0) >> 4;
    uint8_t created_day = ((entry[2] & 0x0f) << 1)
            | ((entry[3] & 0x80) >> 7);
    uint8_t created_year = entry[3] & 0x7f;
    uint8_t created_hour = (entry[4] & 0xf8) >> 3;
    uint8_t created_minute = ((entry[4] & 0x7) << 3)
            | ((entry[5] & 0xe0) >> 5);
    uint8_t created_second = ((entry[5] & 0x1f) << 1)
            | ((entry[6] & 0x80) >> 7);
    uint8_t created_millisecond = entry[6] & 0x7f;

    uint8_t modified_month = (entry[7] & 0xf0) >> 4;
    uint8_t modified_day = ((entry[7] & 0x0f) << 1)
            | ((entry[8] & 0x80) >> 7);
    uint8_t modified_year = entry[8] & 0x7f;
    uint8_t modified_hour = (entry[9] & 0xf8) >> 3;
    uint8_t modified_minute = ((entry[9] & 0x7) << 3)
            | ((entry[10] & 0xe0) >> 5);
    uint8_t modified_second = ((entry[10] & 0x1f) << 1)
            | ((entry[11] & 0x80) >> 7);
    uint8_t modified_millisecond = entry[11] & 0x7f;

    struct directory_entry dir_entry = {
        .parent = parent,
        .reserved = entry[0],
        .attributes = entry[1],

        .created_month = created_month,
        .created_day = created_day,
        .created_year = created_year,
        .created_hour = created_hour,
        .created_minute = created_minute,
        .created_second = created_second,
        .created_millisecond = created_millisecond,

        .modified_month = modified_month,
        .modified_day = modified_day,
        .modified_year = modified_year,
        .modified_hour = modified_hour,
        .modified_minute = modified_minute,
        .modified_second = modified_second,
        .modified_millisecond = modified_millisecond,

        .table_number = get_uint16(entry, 12),
        .first_cluster = get_uint16(entry, 14),
        .file_length = get_uint32(entry, 16)
    };
    dir_entry.filename_entries = entry[20];

    for (size_t i = 0; i < 11; i++) {
        dir_entry.filename[i] = entry[21 + i];
    }

    return (dir_entry);
}

void write_directory_entry(const struct sfs_filesystem sfs,
        struct directory_entry dir_entry) {
    uint8_t entry[32] = { 0 };

    entry[0] = dir_entry.reserved;
    entry[1] = dir_entry.attributes;

    entry[2] = (dir_entry.created_month << 4) | (dir_entry.created_day >> 1);
    entry[3] = (dir_entry.created_day << 7) | (dir_entry.created_year);
    entry[4] = (dir_entry.created_hour << 3)
            | (dir_entry.created_minute >> 3);
    entry[5] = (dir_entry.created_minute << 5)
            | (dir_entry.created_second >> 1);
    entry[6] = (dir_entry.created_second << 7)
            | (dir_entry.created_millisecond);

    entry[7] = (dir_entry.modified_month << 4) | (dir_entry.modified_day >> 1);
    entry[8] = (dir_entry.modified_day << 7) | (dir_entry.modified_year);
    entry[9] = (dir_entry.modified_hour << 3) | (dir_entry.modified_minute >> 3);
    entry[10] = (dir_entry.modified_minute << 5)
            | (dir_entry.modified_second >> 1);
    entry[11] = (dir_entry.modified_second << 7)
            | (dir_entry.modified_millisecond);

    put_uint16(entry, dir_entry.table_number, 12);
    put_uint16(entry, dir_entry.first_cluster, 14);
    put_uint32(entry, dir_entry.file_length, 16);

    entry[20] = dir_entry.filename_entries;
    
    for (size_t i = 0; i < 11; i++) {
        entry[21 + i] = dir_entry.filename[i];
    }

    fwrite(&entry, sizeof(entry), 1, sfs.fp);
}

void move_to_fat(struct sfs_filesystem sfs, uint16_t fat_number) {
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

struct fat_entry get_fat_entry(const struct sfs_filesystem sfs,
        const struct fat_entry entry) {
    move_to_fat(sfs, entry.fat_number);

    FILE* fp = sfs.fp;
    fseek(fp, entry.cluster_number * sizeof(struct fat_entry), SEEK_CUR);

    uint16_t entry_fat_number = read_uint16(fp);
    uint16_t entry_cluster_number = read_uint16(fp);

    struct fat_entry new_entry = {
            .fat_number = entry_fat_number,
            .cluster_number = entry_cluster_number
    };
    return (new_entry);
}

void put_fat_entry(const struct sfs_filesystem sfs,
        const struct fat_entry location, const struct fat_entry entry) {
    move_to_fat(sfs, entry.fat_number);

    FILE* fp = sfs.fp;
    fseek(fp, entry.cluster_number * sizeof(struct fat_entry), SEEK_CUR);

    write_uint16(fp, entry.fat_number);
    write_uint16(fp, entry.cluster_number);
}

void move_to_cluster(const struct sfs_filesystem sfs,
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

uint8_t* read_file_cluster(const struct sfs_filesystem sfs,
        const struct fat_entry entry) {
    move_to_cluster(sfs, entry);

    uint32_t cluster_size = sfs.bytes_per_sector * sfs.sectors_per_cluster;
    uint8_t* cluster = malloc(cluster_size);

    fread(cluster, cluster_size, 1, sfs.fp);
    return (cluster);
}

void write_file_cluster(const struct sfs_filesystem sfs,
        const struct fat_entry entry, uint8_t* cluster) {
    move_to_cluster(sfs, entry);

    uint32_t cluster_size = sfs.bytes_per_sector * sfs.sectors_per_cluster;

    fwrite(cluster, cluster_size, 1, sfs.fp);
}
