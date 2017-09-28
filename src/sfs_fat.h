/*
 * sfs_fat.h
 *
 * Defines functions for reading from and writing to file allocation tables.
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#ifndef SFS_FAT_H
#define SFS_FAT_H

#include "sfs_includes.h"

/**
 * Move the file pointer to an entry in a FAT.
 *
 * @param sfs
 * @param entry the location of the FAT entry to jump to
 * @return success or failure
 */
HIDDEN int sfs_fat_jump_to_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry);

/**
 * Read the value in a FAT entry.
 *
 * @param sfs
 * @param location the location of the entry to read
 * @return the value in the FAT entry
 */
HIDDEN struct fat_entry* sfs_fat_read_fat_entry(
        const struct sfs_filesystem* sfs, const struct fat_entry* location);

/**
 * Write an entry into the FAT.
 *
 * @param sfs
 * @param location the location of the entry to write
 * @param entry the value to write into the entry
 * @return success or failure
 */
HIDDEN int sfs_fat_write_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry* location, const struct fat_entry* entry);

/**
 * Mark a FAT entry as available.
 *
 * @param sfs
 * @param entry the location of the entry to mark
 * @return success or failure
 */
HIDDEN int sfs_fat_mark_as_free(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry);

/**
 * Mark a FAT entry as the end of a chain.
 *
 * @param sfs
 * @param entry the location of the entry to mark
 * @return success or failure
 */
HIDDEN int sfs_fat_mark_as_end_of_chain(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry);

/**
 * Free the FAT entries in the chain following the given entry. Mark the
 * given entry as the end of the chain.
 *
 * @param sfs
 * @param entry the location of the new end-of-the-chain entry
 * @return success or failure
 */
HIDDEN int sfs_fat_truncate_fat_chain(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry);

/**
 * Get the location of the first empty FAT entry after the given entry.
 *
 * @param sfs
 * @param entry the location of the entry at which to start searching
 * @return the location of an empty FAT entry
 */
HIDDEN struct fat_entry* sfs_fat_find_next_empty_fat_entry(
        const struct sfs_filesystem* sfs, const struct fat_entry* entry);

/**
 * Get the location of the first empty FAT entry in the filesystem.
 * <p>
 * NOTE: This function sets the value of sfs->first_available_fat_entry.
 *
 * @param sfs the filesystem within which to search
 * @return the location of the first empty FAT entry
 */
HIDDEN struct fat_entry* sfs_fat_get_first_empty_fat_entry(
        const struct sfs_filesystem* sfs);

/**
 * Mark a chain of available clusters as belonging to the passed file.
 * <p>
 * NOTE: This function stores a reference to the chain and its first
 * cluster in the file struct.
 *
 * @param sfs the filesystem within which to allocate clusters
 * @param file the file being allocated
 * @return success or failure
 */
HIDDEN int sfs_fat_allocate_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file);

/**
 * Allocate a data cluster.
 *
 * @param sfs
 * @param the FAT entry corresponding to the final cluster in the file
 * @return the FAT entry corresponding to the newly allocated cluster
 */
HIDDEN struct fat_entry* sfs_fat_allocate_cluster(
        const struct sfs_filesystem* sfs, const struct fat_entry* end_of_chain);

HIDDEN struct fat_list* sfs_fat_get_cluster_chain(
        const struct sfs_filesystem* sfs, const struct directory_entry* file);

#endif /* SFS_FAT_H */
