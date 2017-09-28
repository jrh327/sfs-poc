/*
 * test_file_functions.c
 *
 *  Created on: Sep 3, 2017
 *      Author: Jon Hopkins
 */

#include "test.h"

int test_create_file() {
    int fd = create_tmp_file();

    const uint16_t fat_size = FAT_SIZE_SMALL;
    const uint16_t bytes_per_sector = 512;
    const uint16_t sectors_per_cluster = 1;
    struct sfs_filesystem* sfs = sfs_initialize_new_filesystem(fd, fat_size,
            bytes_per_sector, sectors_per_cluster);
    struct directory_entry* root = sfs_get_root_directory(sfs);

    const char* filename = "test.txt";
    uint64_t file_length = 52;
    uint8_t* data = allocate(file_length);
    uint8_t* ptr_data = data;
    for (size_t i = 0; i < 26; i++) {
        *ptr_data = (char) ('a' + i);
        ptr_data++;
    }
    for (size_t i = 0; i < 26; i++) {
        *ptr_data = (char) ('A' + i);
        ptr_data++;
    }

    struct directory_entry* file = sfs_create_file(sfs, root, filename, data,
            file_length);

    free(data);

    int ret = 0;
    if (strcmp(file->filename, filename)) {
        printf("filename - expected %s, got %s\n", filename, file->filename);
        ret = -1;
    }
    if (file->first_cluster->fat_number != 0) {
        printf("table number - expected %d, got %d\n", 0,
                file->first_cluster->fat_number);
        ret = -1;
    }
    if (file->first_cluster->cluster_number != 1) {
        printf("first cluster - expected %d, got %d\n", 1,
                file->first_cluster->cluster_number);
        ret = -1;
    }
    if (file->file_length != 52) {
        printf("file length - expected %d, got %d\n", 52, file->file_length);
        ret = -1;
    }

    /*uint64_t file_start = BOOT_SECTOR_SIZE
            + sfs->entries_per_fat * FAT_ENTRY_SIZE
            + sfs->bytes_per_sector * sfs->sectors_per_cluster;
     sfs_util_seek_in_medium(fd, file_start, SEEK_SET);*/
    sfs_file_jump_to_cluster(sfs, file->first_cluster);
    data = allocate(file_length);
    sfs_util_read_from_medium(fd, data, file_length);
    ptr_data = data;
    for (size_t i = 0; i < 26; i++) {
        if (*ptr_data != (char) ('a' + i)) {
            printf("file data - expected %c, got %c\n", (char) ('a' + i),
                    *ptr_data);
            ret = -1;
        }
        ptr_data++;
    }
    for (size_t i = 0; i < 26; i++) {
        if (*ptr_data != (char) ('A' + i)) {
            printf("file data - expected %c, got %c\n", (char) ('A' + i),
                    *ptr_data);
            ret = -1;
        }
        ptr_data++;
    }

    delete_tmp_file();

    return (ret);
}

int test_read_file() {
    int fd = create_tmp_file();

    const uint16_t fat_size = FAT_SIZE_SMALL;
    const uint16_t bytes_per_sector = 512;
    const uint16_t sectors_per_cluster = 1;
    struct sfs_filesystem* sfs = sfs_initialize_new_filesystem(fd, fat_size,
            bytes_per_sector, sectors_per_cluster);
    struct directory_entry* root = sfs_get_root_directory(sfs);

    const char* filename = "test.txt";
    uint64_t file_length = 52;
    uint8_t* data = allocate(file_length);
    uint8_t* ptr_data = data;
    for (size_t i = 0; i < 26; i++) {
        *ptr_data = (char) ('a' + i);
        ptr_data++;
    }
    for (size_t i = 0; i < 26; i++) {
        *ptr_data = (char) ('A' + i);
        ptr_data++;
    }

    struct directory_entry* file = sfs_create_file(sfs, root, filename, data,
            file_length);

    free(data);

    int ret = 0;

    data = allocate(file->file_length);
    sfs_seek_file(sfs, file, 0, SEEK_SET);
    sfs_read_file(sfs, file, data, file->file_length);

    ptr_data = data;
    for (size_t i = 0; i < 26; i++) {
        if (*ptr_data != (char) ('a' + i)) {
            printf("file data - expected %c, got %c\n", (char) ('a' + i),
                    *ptr_data);
            ret = -1;
        }
        ptr_data++;
    }
    for (size_t i = 0; i < 26; i++) {
        if (*ptr_data != (char) ('A' + i)) {
            printf("file data - expected %c, got %c\n", (char) ('A' + i),
                    *ptr_data);
            ret = -1;
        }
        ptr_data++;
    }

    free(data);

    delete_tmp_file();

    return (ret);
}
