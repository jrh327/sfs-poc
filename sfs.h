#ifndef SFS_H
#define SFS_H

#include "includes.h"
#include "files.h"
#include "util.h"

/**
 * Structure containing information stored in the boot sector of the
 * filesystem.
 */
struct boot_sector {
    FILE* fp;
    uint16_t entries_per_fat;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
};

/**
 * Create a new SFS filesystem.
 */
struct boot_sector* initialize_new_filesystem();

/**
 * Read the boot sector of a SFS filesystem.
 * 
 * @param fp the file containing the SFS filesystem
 * @return the boot sector
 */
struct boot_sector* load_filesystem(FILE* fp);

/**
 * Close the filesystem.
 */
void close_filesystem(FILE* fp);

/**
 * Get an entry from a file allocation table.
 * 
 * @param sfs the information in the filesystem's boot sector
 * @param fat_number the number of the allocation table
 * @param entry_number the offset of the entry within the table
 * @return the value in the entry
 */
uint16_t get_fat_entry(struct boot_sector* sfs, uint16_t fat_number,
        uint16_t entry_number);

/**
 * Get the data from a cluster.
 * 
 * @param sfs the information in the filesystem's boot sector
 * @param data_block_number the number of the data block
 * @param cluster_number the offset of the cluster within the data block
 * @return the data in the cluster
 */
uint8_t* get_cluster(struct boot_sector* sfs, uint16_t data_block_number,
        uint16_t cluster_number);

#endif /* SFS_H */
