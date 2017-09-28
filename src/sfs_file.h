/*
 * sfs_file.h
 *
 * Defines functions for reading from and writing to SFS files.
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#ifndef SFS_FILE_H
#define SFS_FILE_H

#include "sfs_includes.h"

/**
 * Move the file pointer to the start of a cluster in a user data block.
 *
 * @param sfs the filesystem in which to move
 * @param cluster FAT entry corresponding to the location of the destination
 * @return success or failure
 */
HIDDEN int sfs_file_jump_to_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry* cluster);

HIDDEN int sfs_file_seek_in_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file, SFS_FILE_OFFSET offset, int whence);

/**
 * Read from disk starting at current file pointer, and decrypt read bytes.
 * <p>
 * This function is meant for reading user data only.
 * <p>
 * This function assumes the file pointer is correctly positioned inside a
 * cluster.
 * <p>
 * This function will handle jumping to the next cluster if necessary.
 * <p>
 * This function will handle alignment with the encryption blocks.
 *
 * @param sfs the filesystem to read from
 * @param file the file to read from
 * @param buffer the array to put the read the data into
 * @param length the number of bytes to read
 * @return success or failure
 */
HIDDEN int sfs_file_read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, uint8_t* buffer,
        const SFS_FILE_LENGTH length);

/**
 * Encrypt bytes and write them to disk, starting at the current file pointer.
 * <p>
 * This function is meant for writing user data only.
 * <p>
 * This function assumes the file pointer is correctly positioned inside a
 * cluster.
 * <p>
 * Tis function will handle jumping to the next cluster if necessary.
 * <p>
 * This function will handle alignment with the encryption blocks.
 *
 * @param sfs the filesystem to write to
 * @param key the encryption key to use to encrypt the written bytes
 * @param data the bytes to write
 * @param clusters list of clusters in file
 * @param length the number of bytes to write
 * @return success or failure
 */
HIDDEN int sfs_file_write_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, const uint8_t* data,
        const SFS_FILE_LENGTH length);

#endif /* SFS_FILE_H */
