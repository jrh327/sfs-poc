#include "files.h"
#include "allocation.h"
#include <time.h>

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

struct directory_entry* create_directory_entry(struct directory_entry* parent,
        const char* filename, const uint64_t file_length) {
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

    time_t t = time(0);
    struct tm* time = gmtime(&t);
    entry->created_month = time->tm_mon + 1;
    entry->created_day = time->tm_mday;
    entry->created_year = time->tm_year + 1900;
    entry->created_hour = time->tm_hour;
    entry->created_minute = time->tm_min;
    entry->created_second = time->tm_sec;
    /* either remove millis or figure out portable way to calculate */
    entry->created_millisecond = 0;

    entry->modified_month = entry->created_month;
    entry->modified_day = entry->created_day;
    entry->modified_year = entry->created_year;
    entry->modified_hour = entry->created_hour;
    entry->modified_minute = entry->created_minute;
    entry->modified_second = entry->created_second;
    entry->modified_millisecond = entry->created_second;

    return (entry);
}

struct directory_entry* read_directory_entry(const struct sfs_filesystem* sfs,
        struct directory_entry* parent) {
    uint8_t entry[DIR_ENTRY_SIZE] = { 0 };
    read_from_file(sfs->fd, &entry, DIR_ENTRY_SIZE);

    int empty_entry = 1;
    for (size_t i = 0; i < DIR_ENTRY_SIZE; i++) {
        if (entry[i] != 0) {
            empty_entry = 0;
            break;
        }
    }

    /* nothing in the entry, don't waste time initializing */
    if (empty_entry) {
        return NULL;
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
            read_from_file(sfs->fd, (extra_entries + index), DIR_ENTRY_SIZE);
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

int jump_to_empty_entry(const struct sfs_filesystem* sfs,
        struct directory_entry* parent) {
    struct fat_list* list = get_file_clusters(sfs, parent);
    struct fat_list* curr = list;

    jump_to_cluster(sfs, *(curr->entry));

    int found = 0;
    int count = 0;
    uint16_t sizeof_cluster = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    while (curr && !found) {
        /* loop through cluster and find an empty entry */
        for (size_t i = 0; i < sizeof_cluster; i += DIR_ENTRY_SIZE) {
            uint8_t* entry = malloc(DIR_ENTRY_SIZE);
            read_from_file(sfs->fd, entry, DIR_ENTRY_SIZE);
            int empty = 1;
            for (size_t j = 0; j < DIR_ENTRY_SIZE; j++) {
                if (entry[j] != 0) {
                    empty = 0;
                    break;
                }
            }
            free(entry);
            if (empty) {
                found = 1;
                /* move back to beginning of the entry */
                seek_in_file(sfs->fd, -DIR_ENTRY_SIZE, SEEK_CUR);
                break;
            }
            count++;
        }

        /* didn't find an empty entry. try next cluster */
        if (!found) {
            curr = curr->next;
            jump_to_cluster(sfs, *(curr->entry));
        }
    }

    /* still no empty entry. need to allocate another cluster */
    if (!found) {
        struct fat_entry entry = allocate_cluster(sfs, list);
        jump_to_cluster(sfs, entry);
    }

    /* done with the parent's cluster list so free it */
    free_fat_list(list);

    return (count);
}

void write_directory_entry(const struct sfs_filesystem* sfs,
        struct directory_entry* dir_entry) {
    uint16_t entry_number = jump_to_empty_entry(sfs, dir_entry->parent);

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

    write_to_file(sfs->fd, &entry, DIR_ENTRY_SIZE);

    /* TODO make sure extra entries are going in the correct cluster */
    uint16_t entries_per_cluster = (sfs->bytes_per_sector
            * sfs->sectors_per_cluster) / DIR_ENTRY_SIZE;
    for (size_t extra = 0; extra < dir_entry->filename_entries; extra++) {
        entry_number++;
        if (entry_number >= entries_per_cluster) {
            struct fat_entry new_location = allocate_cluster(sfs,
                    get_file_clusters(sfs, dir_entry->parent));
            entry_number = 0;
            jump_to_cluster(sfs, new_location);
        }
        entry[0] = extra + 1; /* set the reserved byte to the entry number */
        size_t offset = 11 + extra * (DIR_ENTRY_SIZE - 1);
        size_t index_end = DIR_ENTRY_SIZE; /* not used until the final entry */
        for (size_t i = 1; i < DIR_ENTRY_SIZE; i++) {
            entry[i] = dir_entry->filename[i + offset - 1];
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
        write_to_file(sfs->fd, &entry, DIR_ENTRY_SIZE);
    }
}

void change_file_name(const struct sfs_filesystem* sfs,
        struct directory_entry* file, const char* filename) {
    int filename_length = get_filename_length(filename);
    if (filename_length < 0) {
        filename_length = 1020;
    }
    free(file->filename);
    file->filename = malloc((sizeof(char) * filename_length) + 1);
    memcpy(file->filename, filename, filename_length);
    file->filename[filename_length] = 0;
    file->filename_entries = (filename_length - 11) / (DIR_ENTRY_SIZE - 1);
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

int get_directory_entries(const struct sfs_filesystem* sfs,
        struct directory_entry* parent) {
    struct fat_entry cluster = {
            .fat_number = parent->table_number,
            .cluster_number = parent->first_cluster
    };

    jump_to_cluster(sfs, cluster);

    uint8_t found_end = 0;
    size_t dir_entries_per_cluster = (sfs->bytes_per_sector
            * sfs->sectors_per_cluster) / DIR_ENTRY_SIZE;
    while (!found_end) {
        for (size_t i = 0; i < dir_entries_per_cluster; i++) {
            struct directory_entry* dir_entry = read_directory_entry(sfs,
                    parent);
            if (!dir_entry) {
                found_end = 1;
                break;
            }

            struct directory_list* next = malloc(sizeof(struct directory_list));
            if (!next) {
                return (-1);
            }

            next->entry = dir_entry;
            next->next = parent->contents;
            parent->contents = next;
            return 0;
        }

        /*
         * if directory entries all the way to the end, check if current
         * cluster is not the last one allocated to the directory
         */
        if (!found_end) {
            cluster = get_fat_entry(sfs, cluster);
            if (cluster.fat_number != END_CLUSTER_CHAIN
                    && cluster.cluster_number != END_CLUSTER_CHAIN) {
                jump_to_cluster(sfs, cluster);
            }
        }
    }

    return (0);
}

uint8_t* read_file_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry) {
    jump_to_cluster(sfs, entry);

    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint8_t* cluster = malloc(cluster_size);

    read_from_file(sfs->fd, cluster, cluster_size);
    return (cluster);
}

uint8_t* read_file_clusters(const struct sfs_filesystem* sfs,
        struct fat_list* clusters, const uint64_t file_length) {
    struct fat_list* list_item = clusters;
    uint8_t* data = malloc(file_length);
    uint8_t* ptr_data = data;
    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint64_t remaining = file_length;

    while (list_item && remaining) {
        if (remaining < cluster_size) {
            cluster_size = remaining;
        }
        uint8_t* cluster = read_file_cluster(sfs, *(list_item->entry));
        memcpy(ptr_data, cluster, cluster_size);
        free(cluster);
        ptr_data += cluster_size;
        remaining -= cluster_size;
    }

    return (data);
}

void write_file_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry, const uint8_t* cluster) {
    jump_to_cluster(sfs, entry);

    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;

    write_to_file(sfs->fd, cluster, cluster_size);
}

void write_file_clusters(const struct sfs_filesystem* sfs,
        struct fat_list* clusters, const uint8_t* data,
        const uint64_t file_length) {
    struct fat_list* list_item = clusters;
    uint64_t offset = 0;
    uint32_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint64_t remaining = file_length;

    while (list_item && remaining) {
        if (remaining < cluster_size) {
            uint8_t* cluster = calloc(1, cluster_size);
            memcpy(cluster, data + offset, remaining);
            write_file_cluster(sfs, *(list_item->entry), cluster);
            free(cluster);
            remaining = 0;
        } else {
            write_file_cluster(sfs, *(list_item->entry), data + offset);
            offset += cluster_size;
            remaining -= cluster_size;
        }
        list_item = list_item->next;
    }
}
