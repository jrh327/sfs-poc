#include "files.h"

void move_to_fat(const struct sfs_filesystem* sfs, uint16_t fat_number);
void move_to_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry);

struct directory_entry* read_directory_entry(const struct sfs_filesystem* sfs,
        struct directory_entry* parent) {
    uint8_t entry[DIR_ENTRY_SIZE] = { 0 };
    fread(&entry, sizeof(entry), 1, sfs->fp);

    int empty_entry = 1;
    for (size_t i = 0; i < DIR_ENTRY_SIZE; i++) {
        if (entry[i] != 0) {
            empty_entry = 0;
            break;
        }
    }

    /* nothing in the entry, don't waste time initializing */
    if (empty_entry) {
        return (calloc(1, sizeof(struct directory_entry)));
    }

    uint8_t created_month = (entry[2] & 0xf0) >> 4;
    uint8_t created_day = ((entry[2] & 0x0f) << 1) | ((entry[3] & 0x80) >> 7);
    uint8_t created_year = entry[3] & 0x7f;
    uint8_t created_hour = (entry[4] & 0xf8) >> 3;
    uint8_t created_minute = ((entry[4] & 0x7) << 3) | ((entry[5] & 0xe0) >> 5);
    uint8_t created_second = ((entry[5] & 0x1f) << 1)
            | ((entry[6] & 0x80) >> 7);
    uint8_t created_millisecond = entry[6] & 0x7f;

    uint8_t modified_month = (entry[7] & 0xf0) >> 4;
    uint8_t modified_day = ((entry[7] & 0x0f) << 1) | ((entry[8] & 0x80) >> 7);
    uint8_t modified_year = entry[8] & 0x7f;
    uint8_t modified_hour = (entry[9] & 0xf8) >> 3;
    uint8_t modified_minute = ((entry[9] & 0x7) << 3)
            | ((entry[10] & 0xe0) >> 5);
    uint8_t modified_second = ((entry[10] & 0x1f) << 1)
            | ((entry[11] & 0x80) >> 7);
    uint8_t modified_millisecond = entry[11] & 0x7f;

    struct directory_entry* dir_entry = malloc(sizeof(struct directory_entry));
    dir_entry->parent = parent;
    dir_entry->contents = NULL; /* don't read contents until requested */
    dir_entry->reserved = entry[0];
    dir_entry->attributes = entry[1];

    dir_entry->created_month = created_month;
    dir_entry->created_day = created_day;
    dir_entry->created_year = created_year;
    dir_entry->created_hour = created_hour;
    dir_entry->created_minute = created_minute;
    dir_entry->created_second = created_second;
    dir_entry->created_millisecond = created_millisecond;

    dir_entry->modified_month = modified_month;
    dir_entry->modified_day = modified_day;
    dir_entry->modified_year = modified_year;
    dir_entry->modified_hour = modified_hour;
    dir_entry->modified_minute = modified_minute;
    dir_entry->modified_second = modified_second;
    dir_entry->modified_millisecond = modified_millisecond;

    dir_entry->table_number = get_uint16(entry, 12);
    dir_entry->first_cluster = get_uint16(entry, 14);
    dir_entry->file_length = get_uint32(entry, 16);
    dir_entry->filename_entries = entry[20];

    uint8_t filename_entries = dir_entry->filename_entries;
    if (filename_entries == 0) {
        size_t len;
        for (len = 0; len < 12; len++) {
            if (!entry[21 + len]) {
                break;
            }
        }
        len++; /* one extra for the \0 termination */

        dir_entry->filename = malloc(sizeof(char) * len);
        memcpy(dir_entry->filename, (entry + 21), len - 1);
        dir_entry->filename[len] = 0;
    } else {
        uint8_t* extra_entries = malloc(
                sizeof(char) * (filename_entries * DIR_ENTRY_SIZE));
        size_t index = 0;
        size_t len = 0;
        for (size_t i = 0; i < filename_entries; i++) {
            /* TODO: make sure successive reads are in the correct cluster */
            fread((extra_entries + index), DIR_ENTRY_SIZE, 1, sfs->fp);
            index += DIR_ENTRY_SIZE;
            len += (DIR_ENTRY_SIZE - 1);
        }

        /* go back to the beginning of the last entry and find the \0 */
        index -= DIR_ENTRY_SIZE;
        len -= (DIR_ENTRY_SIZE - 1);

        for (size_t i = 0; i < DIR_ENTRY_SIZE; i++) {
            if (!*(extra_entries + index + i)) {
                break;
            }
            len++;
        }

        /* at this point, index includes the extra byte for \0 termination */
        dir_entry->filename = malloc(sizeof(char) * (len + 11));
        char* filename = dir_entry->filename;
        uint8_t* extra_entry = extra_entries;
        memcpy(filename, (entry + 21), 11);
        filename += 11;
        for (size_t i = 0; i < filename_entries - 1; i++) {
            memcpy(filename, (extra_entry + 1), DIR_ENTRY_SIZE - 1);
            filename += (DIR_ENTRY_SIZE - 1);
            extra_entry += DIR_ENTRY_SIZE;
            len -= (DIR_ENTRY_SIZE - 1);
        }
        memcpy(filename, (extra_entry + 1), len);

        /* just in case the filename extends to the end of the last entry */
        *(filename + len) = 0;

        free(extra_entries);
    }

    return (dir_entry);
}

void write_directory_entry(const struct sfs_filesystem* sfs,
        struct directory_entry* dir_entry) {
    uint8_t entry[DIR_ENTRY_SIZE] = { 0 };

    entry[0] = dir_entry->reserved;
    entry[1] = dir_entry->attributes;

    entry[2] = (dir_entry->created_month << 4) | (dir_entry->created_day >> 1);
    entry[3] = (dir_entry->created_day << 7) | (dir_entry->created_year);
    entry[4] = (dir_entry->created_hour << 3)
            | (dir_entry->created_minute >> 3);
    entry[5] = (dir_entry->created_minute << 5)
            | (dir_entry->created_second >> 1);
    entry[6] = (dir_entry->created_second << 7)
            | (dir_entry->created_millisecond);

    entry[7] = (dir_entry->modified_month << 4)
            | (dir_entry->modified_day >> 1);
    entry[8] = (dir_entry->modified_day << 7) | (dir_entry->modified_year);
    entry[9] = (dir_entry->modified_hour << 3)
            | (dir_entry->modified_minute >> 3);
    entry[10] = (dir_entry->modified_minute << 5)
            | (dir_entry->modified_second >> 1);
    entry[11] = (dir_entry->modified_second << 7)
            | (dir_entry->modified_millisecond);

    put_uint16(entry, dir_entry->table_number, 12);
    put_uint16(entry, dir_entry->first_cluster, 14);
    put_uint32(entry, dir_entry->file_length, 16);

    entry[20] = dir_entry->filename_entries;

    for (size_t i = 0; i < 11; i++) {
        entry[21 + i] = dir_entry->filename[i];
    }
    fwrite(&entry, DIR_ENTRY_SIZE, 1, sfs->fp);

    for (size_t extra = 0; extra < dir_entry->filename_entries; extra++) {
        entry[0] = extra + 1; /* set the reserved byte to the entry number */
        size_t index_end = 0; /* not used until the final entry */
        for (size_t i = 1; i < DIR_ENTRY_SIZE; i++) {
            entry[i] = dir_entry->filename[i + 11];
            if (!entry[i]) {
                /* found the end of the string, break out of loop */
                index_end = i;
                break;
            }
        }
        /* only happens on the last entry, zero out the rest of the entry */
        for (size_t i = index_end; i < DIR_ENTRY_SIZE; i++) {
            entry[i] = 0;
        }
        fwrite(&entry, DIR_ENTRY_SIZE, 1, sfs->fp);
    }
}

struct directory_entry* get_root_directory_entry(
        const struct sfs_filesystem* sfs) {
    struct directory_entry* root = malloc(sizeof(struct directory_entry));

    root->parent = root; /* set root as its own parent */
    root->table_number = 0; /* first file allocation table */
    root->first_cluster = 0; /* first cluster */
    root->file_length = 0; /* directories don't have file length */

    return (root);
}

struct directory_entry* get_directory_entries(const struct sfs_filesystem* sfs,
        struct directory_entry* parent) {
    struct fat_entry fat = {
            .fat_number = parent->table_number,
            .cluster_number = parent->first_cluster
    };

    move_to_cluster(sfs, fat);

    uint8_t found_end = 0;
    size_t dir_entries_per_cluster = (sfs->bytes_per_sector
            * sfs->sectors_per_cluster) / DIR_ENTRY_SIZE;
    while (!found_end) {
        for (size_t i = 0; i < dir_entries_per_cluster; i++) {
            struct directory_entry* dir_entry = read_directory_entry(sfs,
                    parent);
            /* entry is guaranteed to be empty if parent is NULL */
            if (dir_entry->parent == NULL) {
                found_end = 1;
                free(dir_entry);
                break;
            }

            struct directory_list* next = malloc(sizeof(struct directory_list));
            next->entry = dir_entry;
            next->next = parent->contents;
            parent->contents = next;
        }

        /*
         * if directory entries all the way to the end, check if current
         * cluster is not the last one allocated to the directory
         */
        if (!found_end
                && (fat.fat_number != END_CLUSTER_CHAIN
                && fat.cluster_number != END_CLUSTER_CHAIN)) {
            fat = get_fat_entry(sfs, fat);
            move_to_cluster(sfs, fat);
        }
    }

    return (NULL);
}

void move_to_fat(const struct sfs_filesystem* sfs, uint16_t fat_number) {
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
    uint16_t fat_size = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    uint32_t data_block_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    while (fat_number) {
        /* move to the end of the current FAT */
        fseek(fp, fat_size, SEEK_CUR);

        /* move to the end of the current data block */
        fseek(fp, data_block_size, SEEK_CUR);

        fat_number--;
    }
}

struct fat_entry get_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry entry) {
    move_to_fat(sfs, entry.fat_number);

    FILE* fp = sfs->fp;
    fseek(fp, entry.cluster_number * FAT_ENTRY_SIZE, SEEK_CUR);

    uint16_t entry_fat_number = read_uint16(fp);
    uint16_t entry_cluster_number = read_uint16(fp);

    struct fat_entry new_entry = {
            .fat_number = entry_fat_number,
            .cluster_number = entry_cluster_number
    };
    return (new_entry);
}

void put_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry location, const struct fat_entry entry) {
    move_to_fat(sfs, entry.fat_number);

    FILE* fp = sfs->fp;
    fseek(fp, entry.cluster_number * FAT_ENTRY_SIZE, SEEK_CUR);

    write_uint16(fp, entry.fat_number);
    write_uint16(fp, entry.cluster_number);
}

void move_to_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry) {
    /* move to the FAT before the cluster's data block */
    move_to_fat(sfs, entry.cluster_number);

    FILE* fp = sfs->fp;

    /* move to the end of the current FAT */
    fseek(fp, sfs->entries_per_fat * FAT_ENTRY_SIZE, SEEK_CUR);

    if (entry.cluster_number == 0) {
        return;
    }

    /* 
     * loop until the correct cluster is found. cluster number is zero-based,
     * if cluster_number == 0, already at correct cluster and this is skipped
     */
    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint16_t cluster_number = entry.cluster_number;
    while (cluster_number) {
        /* move to the end of the current cluster */
        fseek(fp, cluster_size, SEEK_CUR);

        cluster_number--;
    }
}

uint8_t* read_file_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry) {
    move_to_cluster(sfs, entry);

    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint8_t* cluster = malloc(cluster_size);

    fread(cluster, cluster_size, 1, sfs->fp);
    return (cluster);
}

void write_file_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry, uint8_t* cluster) {
    move_to_cluster(sfs, entry);

    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;

    fwrite(cluster, cluster_size, 1, sfs->fp);
}
