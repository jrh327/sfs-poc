/*
 * test_dir_functions.c
 *
 *  Created on: Sep 3, 2017
 *      Author: Jon Hopkins
 */

#include "test.h"

int test_reading_directory_entry() {
    int fd = create_tmp_file();

    struct sfs_filesystem* sfs = sfs_initialize_new_filesystem(fd,
            FAT_SIZE_SMALL, 512, 1);

    const uint8_t test_reserved = 0;
    const uint8_t test_attributes = 5;

    const uint8_t test_created_month = 3;
    const uint8_t test_created_day = 25;
    const uint8_t test_created_year = 17;
    const uint8_t test_created_hour = 23;
    const uint8_t test_created_minute = 54;
    const uint8_t test_created_second = 13;
    const uint8_t test_created_milli = 75;

    const uint8_t test_modified_month = 3;
    const uint8_t test_modified_day = 26;
    const uint8_t test_modified_year = 17;
    const uint8_t test_modified_hour = 0;
    const uint8_t test_modified_minute = 3;
    const uint8_t test_modified_second = 15;
    const uint8_t test_modified_milli = 23;

    const uint16_t test_table = 1;
    const uint16_t test_cluster = 3;
    const uint32_t test_file_length = 123456;
    const uint8_t test_entries = 0;
    const uint8_t test_filename[11] = { 'f', 'i', 'l', 'e', 'n', 'a', 'm', 'e',
            't', 'x', 't' };

    struct directory_entry* dir_entry = allocate(
            sizeof(struct directory_entry));
    dir_entry->reserved = test_reserved;
    dir_entry->attributes = test_attributes;
    dir_entry->created_month = test_created_month;
    dir_entry->created_day = test_created_day;
    dir_entry->created_year = test_created_year;
    dir_entry->created_hour = test_created_hour;
    dir_entry->created_minute = test_created_minute;
    dir_entry->created_second = test_created_second;
    dir_entry->created_millisecond = test_created_milli;
    dir_entry->modified_month = test_modified_month;
    dir_entry->modified_day = test_modified_day;
    dir_entry->modified_year = test_modified_year;
    dir_entry->modified_hour = test_modified_hour;
    dir_entry->modified_minute = test_modified_minute;
    dir_entry->modified_second = test_modified_second;
    dir_entry->modified_millisecond = test_modified_milli;
    dir_entry->first_cluster = allocate(FAT_ENTRY_SIZE);
    dir_entry->first_cluster->fat_number = test_table;
    dir_entry->first_cluster->cluster_number = test_cluster;
    dir_entry->clusters = allocate(sizeof(struct fat_list));
    dir_entry->clusters->entry = dir_entry->first_cluster;
    dir_entry->file_length = test_file_length;
    dir_entry->filename_entries = test_entries;

    dir_entry->filename = allocate(sizeof(test_filename));
    for (size_t i = 0; i < 11; i++) {
        dir_entry->filename[i] = test_filename[i];
    }

    dir_entry->parent = sfs_get_root_directory(sfs);

    sfs_dir_write_directory_entry(sfs, dir_entry->parent, dir_entry);

    free_directory_entry(dir_entry);
    sfs_util_close_medium(sfs->fd);

    fd = reopen_tmp_file();

    sfs = sfs_load_filesystem(fd);
    sfs->fd = fd;

    struct directory_entry* root = sfs_get_root_directory(sfs);
    /* struct directory_list* dir_list = sfs_list_directory(sfs, root); */

    sfs_file_jump_to_cluster(sfs, root->first_cluster);

    dir_entry = sfs_dir_read_directory_entry(sfs, root); /* first entry in the root */

    sfs_util_close_medium(fd);

    delete_tmp_file();

    int ret = 0;
    if (dir_entry->reserved != test_reserved) {
        printf("reserved - expected %d, got %d\n", test_reserved,
                dir_entry->reserved);
        ret = -1;
    }

    if (dir_entry->attributes != test_attributes) {
        printf("attributes - expected %d, got %d\n", test_attributes,
                dir_entry->attributes);
        ret = -1;
    }

    if (dir_entry->created_month != test_created_month) {
        printf("created_month - expected %d, got %d\n", test_created_month,
                dir_entry->created_month);
        ret = -1;
    }

    if (dir_entry->created_day != test_created_day) {
        printf("created_day - expected %d, got %d\n", test_created_day,
                dir_entry->created_day);
        ret = -1;
    }

    if (dir_entry->created_year != test_created_year) {
        printf("created_year - expected %d, got %d\n", test_created_year,
                dir_entry->created_year);
        ret = -1;
    }

    if (dir_entry->created_hour != test_created_hour) {
        printf("created_hour - expected %d, got %d\n", test_created_hour,
                dir_entry->created_hour);
        ret = -1;
    }

    if (dir_entry->created_minute != test_created_minute) {
        printf("created_minute - expected %d, got %d\n", test_created_minute,
                dir_entry->created_minute);
        ret = -1;
    }

    if (dir_entry->created_second != test_created_second) {
        printf("created_second - expected %d, got %d\n", test_created_second,
                dir_entry->created_second);
        ret = -1;
    }

    if (dir_entry->created_millisecond != test_created_milli) {
        printf("created_millisecond - expected %d, got %d\n",
                test_created_milli, dir_entry->created_millisecond);
        ret = -1;
    }

    if (dir_entry->modified_month != test_modified_month) {
        printf("modified_month - expected %d, got %d\n", test_modified_month,
                dir_entry->modified_month);
        ret = -1;
    }

    if (dir_entry->modified_day != test_modified_day) {
        printf("modified_day - expected %d, got %d\n", test_modified_day,
                dir_entry->modified_day);
        ret = -1;
    }

    if (dir_entry->modified_year != test_modified_year) {
        printf("modified_year - expected %d, got %d\n", test_modified_year,
                dir_entry->modified_year);
        ret = -1;
    }

    if (dir_entry->modified_hour != test_modified_hour) {
        printf("modified_hour - expected %d, got %d\n", test_modified_hour,
                dir_entry->modified_hour);
        ret = -1;
    }

    if (dir_entry->modified_minute != test_modified_minute) {
        printf("modified_minute - expected %d, got %d\n", test_modified_minute,
                dir_entry->modified_minute);
        ret = -1;
    }

    if (dir_entry->modified_second != test_modified_second) {
        printf("modified_second - expected %d, got %d\n", test_modified_second,
                dir_entry->modified_second);
        ret = -1;
    }

    if (dir_entry->modified_millisecond != test_modified_milli) {
        printf("modified_millisecond - expected %d, got %d\n",
                test_modified_milli, dir_entry->modified_millisecond);
        ret = -1;
    }

    if (dir_entry->first_cluster->fat_number != test_table) {
        printf("table_number - expected %d, got %d\n", test_table,
                dir_entry->first_cluster->fat_number);
        ret = -1;
    }

    if (dir_entry->first_cluster->cluster_number != test_cluster) {
        printf("first_cluster - expected %d, got %d\n", test_cluster,
                dir_entry->first_cluster->cluster_number);
        ret = -1;
    }

    if (dir_entry->file_length != test_file_length) {
        printf("file_length - expected %d, got %d\n", test_file_length,
                dir_entry->file_length);
        ret = -1;
    }

    if (dir_entry->filename_entries != test_entries) {
        printf("filename_entries - expected %d, got %d\n", test_entries,
                dir_entry->filename_entries);
        ret = -1;
    }

    for (size_t i = 0; i < 11; i++) {
        if (dir_entry->filename[i] != test_filename[i]) {
            printf("filename[%zu] - expected %c, got %c\n", i, test_filename[i],
                    dir_entry->filename[i]);
            ret = -1;
        }
    }

    return (ret);
}

int test_read_dir_entry_short_filename() {
    int fd = create_tmp_file();

    struct sfs_filesystem* sfs = sfs_initialize_new_filesystem(fd,
            FAT_SIZE_SMALL, 512, 1);
    const uint8_t test_filename[9] = { 'f', 'i', 'l', 'e', '.', 't', 'x', 't',
            '\0' };
    struct directory_entry* dir_entry = allocate(
            sizeof(struct directory_entry));
    dir_entry->filename_entries = 0;
    dir_entry->filename = allocate(sizeof(test_filename));
    for (size_t i = 0; i < 11; i++) {
        dir_entry->filename[i] = test_filename[i];
    }

    dir_entry->file_length = 5;
    sfs_fat_allocate_file(sfs, dir_entry);

    dir_entry->parent = sfs_get_root_directory(sfs);

    sfs_file_jump_to_cluster(sfs, dir_entry->parent->first_cluster);
    sfs_dir_write_directory_entry(sfs, dir_entry->parent, dir_entry);

    free_directory_entry(dir_entry);
    sfs_util_close_medium(fd);

    fd = reopen_tmp_file();

    sfs = sfs_load_filesystem(fd);
    sfs->fd = fd;
    struct directory_entry* root = sfs_get_root_directory(sfs);
    /* struct directory_list* dir_list = sfs_list_directory(sfs, root); */
    sfs_file_jump_to_cluster(sfs, root->first_cluster);
    dir_entry = sfs_dir_read_directory_entry(sfs, root); /* first entry in the root */

    sfs_util_close_medium(fd);

    delete_tmp_file();

    int ret = 0;
    for (size_t i = 0; i < 9; i++) {
        if (dir_entry->filename[i] != test_filename[i]) {
            printf("filename[%zu] - expected %c, got %c\n", i, test_filename[i],
                    dir_entry->filename[i]);
            ret = -1;
        }
    }
    return (ret);
}

int test_read_dir_entry_long_filename() {
    int fd = create_tmp_file();

    struct sfs_filesystem* sfs = sfs_initialize_new_filesystem(fd,
            FAT_SIZE_SMALL, 512, 1);
    const char* test_filename =
            "filenametxtfilenametxtfilenametxtfilenametxtfilenametxtfilenametxt";
    struct directory_entry* dir_entry = allocate(
            sizeof(struct directory_entry));
    dir_entry->filename_entries = 2;
    size_t len = strlen(test_filename);
    dir_entry->filename = allocate(len + 1);
    for (size_t i = 0; i < len; i++) {
        dir_entry->filename[i] = test_filename[i];
    }
    dir_entry->filename[len] = 0;

    dir_entry->file_length = 5;
    sfs_fat_allocate_file(sfs, dir_entry);

    dir_entry->parent = sfs_get_root_directory(sfs);

    sfs_file_jump_to_cluster(sfs, &FREE_ENTRY);
    sfs_dir_write_directory_entry(sfs, dir_entry->parent, dir_entry);

    free_directory_entry(dir_entry);
    sfs_util_close_medium(fd);

    fd = reopen_tmp_file();

    sfs->fd = fd;
    struct directory_entry* root = sfs_get_root_directory(sfs);
    /* struct directory_list* dir_list = sfs_list_directory(sfs, root); */
    sfs_file_jump_to_cluster(sfs, &FREE_ENTRY);
    dir_entry = sfs_dir_read_directory_entry(sfs, root); /* first entry in the root */

    int ret = 0;

    /* make sure reserved bytes are set correctly */
    sfs_util_seek_in_medium(fd,
            BOOT_SECTOR_SIZE + sfs->entries_per_fat * FAT_ENTRY_SIZE, SEEK_SET);
    for (size_t i = 0; i < 3; i++) {
        uint8_t entry[DIR_ENTRY_SIZE] = { 0 };
        sfs_util_read_from_medium(fd, &entry, DIR_ENTRY_SIZE);
        if (entry[0] != i) {
            printf("entry %zu - expected reserved byte to be %zu, got %d\n", i,
                    i, entry[0]);
            ret = -1;
        }
    }
    sfs_util_close_medium(fd);

    /*delete_tmp_file();*/

    char* filename = dir_entry->filename;
    for (size_t i = 0; i < len; i++) {
        if (*filename != test_filename[i]) {
            printf("filename[%zu] - expected %c, got %c\n", i, test_filename[i],
                    *filename);
            ret = -1;
        }
        filename++;
    }
    return (ret);
}
