/*
 * test_sfs_functions.c
 *
 *  Created on: Sep 3, 2017
 *      Author: Jon Hopkins
 */

#include "test.h"

int test_new_bootsector_constraints() {
    int fd = create_tmp_file();

    uint16_t init_fat_size = FAT_SIZE_MEDIUM;
    uint16_t init_bytes_per_sector = 512;
    uint8_t init_sectors_per_cluster = 64;
    uint16_t expected_fat_size = FAT_SIZE_MEDIUM;
    uint16_t expected_bytes_per_sector = 512;
    uint8_t expected_sectors_per_cluster = 64;
    struct sfs_filesystem* sfs = sfs_initialize_filesystem_partition(fd, 0,
            init_fat_size, init_bytes_per_sector, init_sectors_per_cluster);
    int ret = 0;
    if (sfs->entries_per_fat != expected_fat_size) {
        printf("entries_per_fat 1 - expected %d, got %d\n", expected_fat_size,
                sfs->entries_per_fat);
        ret = -1;
    }

    if (sfs->bytes_per_sector != expected_bytes_per_sector) {
        printf("bytes_per_sector 1 - expected %d, got %d\n",
                expected_bytes_per_sector, sfs->bytes_per_sector);
        ret = -1;
    }

    if (sfs->sectors_per_cluster != expected_sectors_per_cluster) {
        printf("sectors_per_cluster 1 - expected %d, got %d\n",
                expected_sectors_per_cluster, sfs->sectors_per_cluster);
        ret = -1;
    }

    sfs_util_close_medium(fd);

    delete_tmp_file();

    fd = create_tmp_file();

    init_fat_size = FAT_SIZE_MEDIUM - 1;
    init_bytes_per_sector = 500;
    init_sectors_per_cluster = 128;
    expected_fat_size = FAT_SIZE_MEDIUM; /* default */
    expected_bytes_per_sector = 512; /* minimum */
    expected_sectors_per_cluster = 64; /* 32K byte max cluster size */
    sfs = sfs_initialize_filesystem_partition(fd, 0, init_fat_size,
            init_bytes_per_sector, init_sectors_per_cluster);
    if (sfs->entries_per_fat != expected_fat_size) {
        printf("entries_per_fat 2 - expected %d, got %d\n", expected_fat_size,
                sfs->entries_per_fat);
        ret = -1;
    }

    if (sfs->bytes_per_sector != expected_bytes_per_sector) {
        printf("bytes_per_sector 2 - expected %d, got %d\n",
                expected_bytes_per_sector, sfs->bytes_per_sector);
        ret = -1;
    }

    if (sfs->sectors_per_cluster != expected_sectors_per_cluster) {
        printf("sectors_per_cluster 2 - expected %d, got %d\n",
                expected_sectors_per_cluster, sfs->sectors_per_cluster);
        ret = -1;
    }

    sfs_util_close_medium(fd);

    delete_tmp_file();

    fd = create_tmp_file();

    init_fat_size = FAT_SIZE_MEDIUM;
    init_bytes_per_sector = 544;
    init_sectors_per_cluster = 20;
    expected_fat_size = FAT_SIZE_MEDIUM; /* default */
    expected_bytes_per_sector = 512; /* uses MSB if not power of 2 */
    expected_sectors_per_cluster = 16; /* uses MSB if not power of 2 */
    sfs = sfs_initialize_filesystem_partition(fd, 0, init_fat_size,
            init_bytes_per_sector, init_sectors_per_cluster);
    if (sfs->entries_per_fat != expected_fat_size) {
        printf("entries_per_fat 3 - expected %d, got %d\n", expected_fat_size,
                sfs->entries_per_fat);
        ret = -1;
    }

    if (sfs->bytes_per_sector != expected_bytes_per_sector) {
        printf("bytes_per_sector 3 - expected %d, got %d\n",
                expected_bytes_per_sector, sfs->bytes_per_sector);
        ret = -1;
    }

    if (sfs->sectors_per_cluster != expected_sectors_per_cluster) {
        printf("sectors_per_cluster 3 - expected %d, got %d\n",
                expected_sectors_per_cluster, sfs->sectors_per_cluster);
        ret = -1;
    }

    sfs_util_close_medium(fd);

    delete_tmp_file();

    return (ret);
}
