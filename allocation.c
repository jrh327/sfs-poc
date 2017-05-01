#include "includes.h"
#include "structs.h"
#include "util.h"

void jump_to_fat(const struct sfs_filesystem* sfs, uint16_t fat_number) {
    uint64_t begin_data = BOOT_SECTOR_SIZE;
    uint64_t sizeof_fat = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    uint64_t sizeof_data_block = sfs->entries_per_fat
            * sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint64_t location = begin_data
            + fat_number * (sizeof_fat + sizeof_data_block);

    FILE* fp = sfs->fp;

    /* check if going past the current end of the filesystem */
    fseek(fp, 0, SEEK_END);
    uint64_t size = ftell(fp);
    fseek(fp, location, SEEK_SET);

    /* generate the FAT if we went past the end of the existing filesystem */
    if (size < location) {
        uint8_t* fat = calloc(1, sizeof_fat);
        fwrite(fat, sizeof_fat, 1, fp);
        free(fat);
    }
}

void jump_to_cluster(const struct sfs_filesystem* sfs, struct fat_entry entry)  {
    jump_to_fat(sfs, entry.fat_number);

    uint64_t sizeof_fat = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    uint64_t sizeof_cluster = sfs->bytes_per_sector * sfs->sectors_per_cluster;

    /* using ftell to get location relative to the beginning of the filesystem
     * instead of relative to the position set by the call to jump_to_fat() */
    uint64_t cluster_location = ftell(sfs->fp) + sizeof_fat
            + sizeof_cluster * entry.cluster_number;

    FILE* fp = sfs->fp;

    /* check if going past the current end of the filesystem */
    fseek(fp, 0, SEEK_END);
    uint64_t size = ftell(fp);
    fseek(fp, cluster_location, SEEK_SET);

    /* generate the cluster if we went past the end of the existing filesystem */
    if (size <= cluster_location) {
        uint8_t* cluster = calloc(1, sizeof_cluster);
        fwrite(cluster, sizeof_cluster, 1, fp);
        free(cluster);

        /* rewind to beginning of cluster */
        fseek(fp, cluster_location, SEEK_SET);
    }
}

struct fat_entry find_next_avail_fat_entry(const struct sfs_filesystem* sfs,
        struct fat_entry start) {
    FILE* fp = sfs->fp;
    uint16_t entries_per_fat = sfs->entries_per_fat;

    uint16_t fat_number = start.fat_number;
    uint16_t cluster_number = start.cluster_number;
    jump_to_fat(sfs, fat_number);
    fseek(sfs->fp, cluster_number * FAT_ENTRY_SIZE, SEEK_CUR);

    int found = 0;
    while (!found) {
        uint16_t entry_fat_number = read_uint16(fp);
        uint16_t entry_cluster_number = read_uint16(fp);

        if (entry_fat_number == 0 && entry_cluster_number == 0) {
            found = 1;
            break;
        }

        cluster_number++;
        if (cluster_number == entries_per_fat) {
            fat_number++;
            cluster_number = 0;
            jump_to_fat(sfs, fat_number);
        }
    }

    struct fat_entry available = {
            .fat_number = fat_number,
            .cluster_number = cluster_number
    };

    return (available);
}

struct fat_entry get_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry entry) {
    jump_to_fat(sfs, entry.fat_number);

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
    jump_to_fat(sfs, location.fat_number);

    FILE* fp = sfs->fp;
    fseek(fp, location.cluster_number * FAT_ENTRY_SIZE, SEEK_CUR);

    write_uint16(fp, entry.fat_number);
    write_uint16(fp, entry.cluster_number);
}

struct fat_entry allocate_cluster(const struct sfs_filesystem* sfs,
        struct fat_list* list) {
    struct fat_list* curr = list;
    while (curr->next) {
        curr = curr->next;
    }

    struct fat_entry end_chain = {
            .fat_number = END_CLUSTER_CHAIN,
            .cluster_number = END_CLUSTER_CHAIN
    };
    struct fat_entry entry = find_next_avail_fat_entry(sfs, *(curr->entry));
    put_fat_entry(sfs, *(curr->entry), entry);
    put_fat_entry(sfs, entry, end_chain);

    return (entry);
}

struct fat_list* allocate_file(const struct sfs_filesystem* sfs,
        uint64_t file_len) {
    struct fat_entry entry = sfs->first_available_fat_entry;
    uint64_t sizeof_cluster = sfs->bytes_per_sector * sfs->sectors_per_cluster;

    struct fat_list* list = malloc(sizeof(struct fat_list));
    list->entry = malloc(sizeof(struct fat_entry));
    list->entry->fat_number = entry.fat_number;
    list->entry->cluster_number = entry.cluster_number;
    list->next = NULL;

    struct fat_list* tail = list;

    struct fat_entry end_chain = {
            .fat_number = END_CLUSTER_CHAIN,
            .cluster_number = END_CLUSTER_CHAIN
    };

    put_fat_entry(sfs, entry, end_chain);

    while (file_len > sizeof_cluster) {
        struct fat_entry next = find_next_avail_fat_entry(sfs, entry);
        put_fat_entry(sfs, entry, next);
        put_fat_entry(sfs, next, end_chain);

        tail->next = malloc(sizeof(struct fat_list));
        tail = tail->next;
        tail->entry = malloc(FAT_ENTRY_SIZE);
        tail->entry->fat_number = next.fat_number;
        tail->entry->cluster_number = next.cluster_number;
        tail->next = NULL;

        entry = next;
        file_len -= sizeof_cluster;
    }

    /* make sure to find the next available entry */
    find_next_avail_fat_entry(sfs, entry);

    return (list);
}

struct fat_list* get_file_clusters(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    struct fat_list* list = malloc(sizeof(struct fat_list));
    list->entry = malloc(FAT_ENTRY_SIZE);
    list->entry->fat_number = file->table_number;
    list->entry->cluster_number = file->first_cluster;
    struct fat_list* tail = list;

    while (tail->entry->fat_number != END_CLUSTER_CHAIN
            || tail->entry->cluster_number != END_CLUSTER_CHAIN) {
        struct fat_entry next = get_fat_entry(sfs, *(tail->entry));
        tail->next = malloc(sizeof(struct fat_list));
        tail = tail->next;
        tail->next = NULL;
        tail->entry = malloc(FAT_ENTRY_SIZE);
        tail->entry->fat_number = next.fat_number;
        tail->entry->cluster_number = next.cluster_number;
    }

    return (list);
}
