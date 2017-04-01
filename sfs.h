#ifndef SFS_H
#define SFS_H

#include "includes.h"
#include "structs.h"
#include "files.h"
#include "util.h"

/**
 * Create a new SFS filesystem with no partition offset.
 * 
 * Note: this is the same as calling initialize_filesystem_partition()
 * with partition_offset set to 0.
 * 
 * @param fp the file that will contain the SFS filesystem
 * @param fat_size the number of entries in the file allocation table
 * @param bytes_per_sector the number of bytes in a data sector
 * @param sectors_per_cluster the number of sectors in a data cluster
 * @return the newly created filesystem
 */
struct sfs_filesystem initialize_new_filesystem(FILE* fp, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster);

/**
 * Create a new SFS filesystem partition at the given offset.
 * 
 * @param fp the file that will contain the SFS filesystem
 * @param partition_offset how far into fp to start the filesystem
 * @param fat_size the number of entries in the file allocation table
 * @param bytes_per_sector the number of bytes in a data sector
 * @param sectors_per_cluster the number of sectors in a data cluster
 * @return the newly created filesystem
 */
struct sfs_filesystem initialize_filesystem_partition(FILE* fp,
        uint64_t partition_offset, uint16_t fat_size, uint16_t bytes_per_sector,
        uint8_t sectors_per_cluster);

/**
 * Read the boot sector of a SFS filesystem.
 * 
 * @param fp the file containing the SFS filesystem
 * @return the filesystem
 */
struct sfs_filesystem load_filesystem(FILE* fp);

/**
 * Close the filesystem.
 * 
 * @param sfs the filesystem
 */
void close_filesystem(struct sfs_filesystem sfs);

#endif /* SFS_H */
