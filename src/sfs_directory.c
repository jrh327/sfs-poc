/*
 * sfs_directory.c
 *
 *  Created on: Aug 19, 2017
 *      Author: Jon Hopkins
 */

#include "sfs_includes.h"

HIDDEN struct directory_entry* sfs_dir_get_root_directory_entry(
        const struct sfs_filesystem* sfs) {
    struct directory_entry* root = allocate(sizeof(struct directory_entry));
    root->first_cluster = allocate(FAT_ENTRY_SIZE);
    return (root);
}

HIDDEN struct directory_list* sfs_dir_get_directory_contents(
        const struct sfs_filesystem* sfs, const struct directory_entry* dir) {
    return (NULL);
}

HIDDEN struct file_stat* sfs_dir_describe_file(const struct sfs_filesystem* sfs,
        const int fd) {
    return (NULL);
}

HIDDEN struct file_stat* sfs_dir_describe_file_in_directory(
        const struct sfs_filesystem* sfs, const int dir, const char* filename) {
    return (NULL);
}

static int sfs_dir_get_filename_length(const char* filename) {
    /*
     * check how many bytes the current character spans.
     *
     * 1 byte for ascii characters
     * 2-byte code points' first byte like 0b110xxxxx
     * 3-byte code points' first byte like 0b1110xxxx
     * 4-byte code points' first byte like 0b11110xxx
     * remaining bytes in code point like 0b10xxxxxx
     *
     * don't waste time checking for malformed unicode;
     * let the client deal with that; only constraining
     * on byte- and character-length here.
     * character-length can be determined by counting the
     * bytes that don't have first two bits set to 10.
     * ascii characters will never have top bit set, and
     * multi-byte code points' first byte will have at
     * least the top two bits set, while the rest have top
     * two bits set to 10 to indicate continuation of the
     * code point.
     *
     * https://en.wikipedia.org/wiki/UTF-8#Description
     * https://stackoverflow.com/a/7298149
     */
    size_t len = 0;
    for (size_t i = 0; i < 1024; i++) { /* limit to 1024 bytes */
        if (!filename[i]) {
            break; /* found null terminator */
        }
        /* https://stackoverflow.com/a/7298458 */
        if ((filename[i] & 0xC0) != 0x80) {
            ++len;
        }
    }
    return (len);
}

HIDDEN struct directory_entry* sfs_dir_create_directory_entry(
        struct directory_entry* parent, const char* filename,
        const uint64_t file_length) {
    struct directory_entry* entry = allocate(sizeof(struct directory_entry));
    int filename_length = sfs_dir_get_filename_length(filename);
    if (filename_length > 0) {
        entry->filename = allocate((sizeof(char) * filename_length) + 1);
        memcpy(entry->filename, filename, filename_length);
    } else {
        filename_length = 1020;
        entry->filename = allocate((sizeof(char) * filename_length) + 1);
        memcpy(entry->filename, filename, filename_length);
    }
    entry->filename[filename_length] = 0;
    entry->filename_entries = (filename_length - 11) / (DIR_ENTRY_SIZE - 1);

    entry->reserved = 0;
    entry->parent = parent;
    entry->file_length = file_length;

    /* TODO need to decide how to set attributes */
    entry->attributes = 0;

    time_t t = time(0);
    struct tm* time = gmtime(&t);
    entry->created_month = time->tm_mon + 1;
    entry->created_day = time->tm_mday;
    entry->created_year = time->tm_year + 1900;
    entry->created_hour = time->tm_hour;
    entry->created_minute = time->tm_min;
    entry->created_second = time->tm_sec;
    /* TODO either remove millis or figure out portable way to calculate */
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

HIDDEN struct directory_entry* sfs_dir_read_directory_entry(
        const struct sfs_filesystem* sfs, struct directory_entry* parent) {
    uint8_t entry[DIR_ENTRY_SIZE] = { 0 };
    sfs_util_read_from_medium(sfs->fd, &entry, DIR_ENTRY_SIZE);

    /* nothing in the entry, don't waste time initializing */
    if (sfs_util_empty(entry, DIR_ENTRY_SIZE)) {
        return (NULL);
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

    struct directory_entry* dir_entry = allocate(
            sizeof(struct directory_entry));
    dir_entry->parent = parent;
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

    struct fat_entry* first_cluster = allocate(FAT_ENTRY_SIZE);
    first_cluster->fat_number = sfs_util_get_uint16(entry, 12);
    first_cluster->cluster_number = sfs_util_get_uint16(entry, 14);
    dir_entry->first_cluster = first_cluster;
    dir_entry->file_length = sfs_util_get_uint32(entry, 16);
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

        dir_entry->filename = allocate(sizeof(char) * len);
        memcpy(dir_entry->filename, (entry + 21), len - 1);
        dir_entry->filename[len] = 0;
    } else {
        uint8_t* extra_entries = allocate(
                sizeof(char) * (filename_entries * DIR_ENTRY_SIZE));
        size_t index = 0;
        size_t len = 0;
        for (size_t i = 0; i < filename_entries; i++) {
            /* TODO: make sure successive reads are in the correct cluster */
            sfs_util_read_from_medium(sfs->fd, (extra_entries + index),
                    DIR_ENTRY_SIZE);
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
        dir_entry->filename = allocate(sizeof(char) * (len + 11));
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

static int sfs_dir_find_entry_slot(const struct sfs_filesystem* sfs,
        const struct directory_entry* parent, int spaces) {
    return (0);
}

HIDDEN int sfs_dir_write_directory_entry(const struct sfs_filesystem* sfs,
        const struct directory_entry* parent, struct directory_entry* entry) {
    /*
     - loop scan directory entries until a space large enough to fit the name is found
     - store current position
     - set flag true
     - loop for 1 to number extra entries
     -   if non-empty entry, set flag false and break
     -   if entry contains nothing but 0s, end of used entries, can't have non-empty entries after, so break
     -   if end of directory's current allocation, can't have any non-empty entries after, so break
     - if flag is true
     -   create directory entry
     -   write position in parent into directory entry
     -   write directory entry
     -   allocate file, store returned FAT chain, write first FAT entry to directory entry
     -   write file
     - else
     -   continue - reading will place us at the next directory entry anyway
     - loop for entry in FAT chain
     -   jump to entry's location
     -   write contents to cluster
     return the directory entry of the newly created file
     */
    /*
     struct fat_entry start = {
     .fat_number = parent->table_number,
            .cluster_number = parent->first_cluster
    };
    sfs_file_jump_to_cluster(sfs, start);
    int entry_number = 0;
    int found = 0;
     uint8_t* entry_bytes = allocate(DIR_ENTRY_SIZE);
    while (!found) {
        if (sfs_file_read_file_section(sfs, DIR_ENTRY_SIZE)) {

        }
    }
     */
    sfs_file_jump_to_cluster(sfs, parent->first_cluster);
    int entry_number = sfs_dir_find_entry_slot(sfs, parent,
            entry->filename_entries + 1);

    uint8_t entry_bytes[DIR_ENTRY_SIZE] = { 0 };

    entry_bytes[0] = entry->reserved;
    entry_bytes[1] = entry->attributes;

    entry_bytes[2] = (entry->created_month << 4) | (entry->created_day >> 1);
    entry_bytes[3] = (entry->created_day << 7) | (entry->created_year);
    entry_bytes[4] = (entry->created_hour << 3) | (entry->created_minute >> 3);
    entry_bytes[5] = (entry->created_minute << 5)
            | (entry->created_second >> 1);
    entry_bytes[6] = (entry->created_second << 7)
            | (entry->created_millisecond);

    entry_bytes[7] = (entry->modified_month << 4) | (entry->modified_day >> 1);
    entry_bytes[8] = (entry->modified_day << 7) | (entry->modified_year);
    entry_bytes[9] = (entry->modified_hour << 3)
            | (entry->modified_minute >> 3);
    entry_bytes[10] = (entry->modified_minute << 5)
            | (entry->modified_second >> 1);
    entry_bytes[11] = (entry->modified_second << 7)
            | (entry->modified_millisecond);

    sfs_util_put_uint16(entry_bytes, entry->first_cluster->fat_number, 12);
    sfs_util_put_uint16(entry_bytes, entry->first_cluster->cluster_number, 14);
    sfs_util_put_uint32(entry_bytes, entry->file_length, 16);

    entry_bytes[20] = entry->filename_entries;

    for (size_t i = 0; i < 11; i++) {
        entry_bytes[21 + i] = entry->filename[i];
    }

    int loc = sfs_util_seek_in_medium(sfs->fd, 0, SEEK_CUR);
    sfs_util_write_to_medium(sfs->fd, &entry_bytes, DIR_ENTRY_SIZE);

    /* TODO make sure extra entries are going in the correct cluster */
    uint16_t entries_per_cluster = (sfs->bytes_per_sector
            * sfs->sectors_per_cluster) / DIR_ENTRY_SIZE;
    for (size_t extra = 0; extra < entry->filename_entries; extra++) {
        entry_number++;
        if (entry_number >= entries_per_cluster) {
            struct fat_entry* new_location = sfs_fat_allocate_cluster(sfs,
                    entry->parent->current_cluster->entry);
            entry_number = 0;
            sfs_file_jump_to_cluster(sfs, new_location);
        }
        entry_bytes[0] = extra + 1; /* set the reserved byte to the entry number */
        entry_bytes[0] |= entry->reserved; /* apply any mask in reserved byte */
        size_t offset = 11 + extra * (DIR_ENTRY_SIZE - 1);
        size_t index_end = DIR_ENTRY_SIZE; /* not used until the final entry */
        for (size_t i = 1; i < DIR_ENTRY_SIZE; i++) {
            entry_bytes[i] = entry->filename[i + offset - 1];
            if (!entry_bytes[i]) {
                /* found the end of the string, break out of loop */
                index_end = i;
                break;
            }
        }
        /* only happens on the last entry, zero out the rest of the entry */
        for (size_t i = index_end; i < DIR_ENTRY_SIZE; i++) {
            entry_bytes[i] = 0;
        }
        loc = sfs_util_seek_in_medium(sfs->fd, 0, SEEK_CUR);
        sfs_util_write_to_medium(sfs->fd, &entry_bytes, DIR_ENTRY_SIZE);
    }

    return (0);
}

HIDDEN int sfs_dir_update_directory_entry(const struct sfs_filesystem* sfs,
        const struct directory_entry* entry) {

    for (int i = 0; i < entry->filename_entries; i++) {
        /* get next filename entry */
        /* filename_entry->reserved &= 0x3f; */

    }
    return (0);
}

HIDDEN int sfs_dir_change_file_name(const struct sfs_filesystem* sfs,
        struct directory_entry* file, const char* filename) {
    return (0);
}
