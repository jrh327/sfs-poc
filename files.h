#ifndef FILES_H
#define FILES_H

#include "includes.h"
#include "structs.h"
#include "util.h"

/**
 * Read a directory entry from the given file.
 * 
 * @param sfs the filesystem to read from
 * @return the directory entry
 */
struct directory_entry read_directory_entry(const struct sfs_filesystem sfs);

/**
 * Write a directory entry to the given file.
 * 
 * @param sfs the filesystem to write to
 * @param dir_entry the directory entry to write
 */
void write_directory_entry(const struct sfs_filesystem sfs,
        struct directory_entry dir_entry);

/**
 * Get an entry from a file allocation table.
 * 
 * @param sfs the filesystem to read from
 * @param entry the location of the entry to retrieve
 * @return the value in the entry
 */
struct fat_entry get_fat_entry(const struct sfs_filesystem sfs,
        const struct fat_entry entry);
/**
 * Get an entry from a file allocation table.
 * 
 * @param sfs the filesystem to read from
 * @param location the location of the entry to write
 * @param entry the entry to write
 */
void put_fat_entry(const struct sfs_filesystem sfs,
        const struct fat_entry location, const struct fat_entry entry);

/**
 * Get the data from a cluster.
 * 
 * @param sfs the filesystem to read from
 * @param entry the location of the cluster to retrieve
 * @return the data in the cluster
 */
uint8_t* read_file_cluster(const struct sfs_filesystem sfs,
        const struct fat_entry entry);

/**
 * Read a cluster from the filesystem.
 * 
 * @param sfs the filesystem to write to
 * @param entry the location to write to
 * @param cluster the bytes of the cluster
 */
void write_file_cluster(const struct sfs_filesystem sfs,
        const struct fat_entry entry, uint8_t* cluster);

#endif /* FILES_H */
