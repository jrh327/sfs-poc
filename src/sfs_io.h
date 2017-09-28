/*
 * sfs_io.h
 *
 * Defines functions for reading from and writing to the underlying medium.
 * These functions handle encryption.
 *
 *  Created on: Aug 22, 2017
 *      Author: Jon Hopkins
 */

#ifndef SFS_IO_H
#define SFS_IO_H

#include "sfs_includes.h"

/**
 * Read from disk at the given FAT entry, and decrypt read bytes.
 * <p>
 * This function is meant for reading FAT entries only.
 * <p>
 * This function assumes the file pointer is already pointing to the desired
 * FAT entry.
 * <p>
 * This function will handle alignment with the encryption blocks.
 *
 * @param sfs the filesystem to read from
 * @return the FAT entry pointed to by the read entry
 */
HIDDEN struct fat_entry* sfs_io_read_fat_entry(const struct sfs_filesystem *sfs);

/**
 * Encrypt bytes and write them to disk, starting at the given FAT entry.
 * <p>
 * This function is meant for writing FAT entries only.
 * <p>
 * This function assumes the file pointer is already pointing to the desired
 * FAT entry.
 * <p>
 * This function will handle alignment with the encryption blocks.
 *
 * @param sfs the filesystem to write to
 * @param entry the value to write into the FAT entry
 * @return success or failure
 */
HIDDEN int sfs_io_write_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry);

/**
 * Write a new FAT table at the end of the filesystem.
 * <p>
 * This function assumes the file pointer is already pointing to the start
 * of the new FAT.
 *
 * @param sfs the filesystem to write to
 * @return success or failure
 */
HIDDEN int sfs_io_write_new_fat(const struct sfs_filesystem* sfs);

/**
 * Read the cluster, starting from the current position, to the end of the
 * cluster, or up until `length` bytes.
 * <p>
 * This function assumes the file pointer is already pointing to the desired
 * position within the cluster.
 * <p>
 * This function will handle alignment with the encryption blocks.
 *
 * @return the number of bytes read
 */
HIDDEN int sfs_io_read_cluster(const struct sfs_filesystem* sfs,
        const struct encryption_key* key, uint8_t* buffer,
        const SFS_FILE_LENGTH length);

/**
 * Write to the cluster, starting from the current position, to the end of the
 * cluster, or up until `length` bytes.
 * <p>
 * This function assumes the file pointer is already pointing to the desired
 * position within the cluster.
 * <p>
 * This function will handle alignment with the encryption blocks.
 *
 * @return the number of bytes written
 */
HIDDEN int sfs_io_write_cluster(const struct sfs_filesystem* sfs,
        const struct encryption_key* key, const uint8_t* data,
        const SFS_FILE_LENGTH length);

/**
 * Write a new cluster at the end of the filesystem.
 * <p>
 * This function assumes the file pointer is already pointing to the start
 * of the new cluster.
 *
 * @param sfs the filesystem to write to
 * @return success or failure
 */
HIDDEN int sfs_io_write_new_cluster(const struct sfs_filesystem* sfs);

#endif /* SFS_IO_H */
