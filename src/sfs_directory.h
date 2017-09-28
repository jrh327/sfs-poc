/*
 * sfs_directory.h
 *
 * Defines functions for retrieving files from and putting files into SFS
 * directories, as well as renaming files.
 *
 *  Created on: Aug 19, 2017
 *      Author: Jon Hopkins
 */

#ifndef SFS_DIRECTORY_H
#define SFS_DIRECTORY_H

#include "sfs_includes.h"

/**
 * Get the root directory of the filesystem.
 *
 * @param sfs the filesystem whose root is being retrieved
 * @return the root directory entry of the filesystem
 */
HIDDEN struct directory_entry* sfs_dir_get_root_directory_entry(
        const struct sfs_filesystem* sfs);

/**
 * Get the contents of a directory.
 *
 * @param sfs the filesystem in which the directory is located
 * @param dir the directory whose contents are being retrieved
 * @return a list of entries of the contents of the directory
 */
HIDDEN struct directory_list* sfs_dir_get_directory_contents(
        const struct sfs_filesystem* sfs, const struct directory_entry* dir);

/**
 * Get information about an open file.
 *
 * @param sfs the filesystem in which the file resides
 * @param fd the file descriptor of the file
 * @return the file's information
 */
HIDDEN struct file_stat* sfs_dir_describe_file(const struct sfs_filesystem* sfs,
        const int fd);

/**
 * Get information about a file by filename.
 *
 * @param sfs the filesystem in which the file resides
 * @param dir the file descriptor of the file's parent directory
 * @param filename the name of the file to describe
 * @return the file's information
 */
HIDDEN struct file_stat* sfs_dir_describe_file_in_directory(
        const struct sfs_filesystem* sfs, const int dir, const char* filename);

/**
 * Create a directory entry for a file with the given attributes.
 * <p>
 * This function does not write anything to the filesystem.
 *
 * @param parent the directory in which the file will reside
 * @param filename the name of the file
 * @param file_length the number of bytes in the file
 * @return the directory entry
 */
HIDDEN struct directory_entry* sfs_dir_create_directory_entry(
        struct directory_entry* parent, const char* filename,
        const uint64_t file_length);

/**
 * Read an entry from a directory.
 * <p>
 * This function assumes the file handle is already pointing to
 * the desired entry.
 *
 * @param sfs the filesystem to read from
 * @param parent the directory entry where the entry is located
 * @return the directory entry
 */
HIDDEN struct directory_entry* sfs_dir_read_directory_entry(
        const struct sfs_filesystem* sfs, struct directory_entry* parent);

/**
 * Write an entry into a directory.
 * <p>
 * This function handles finding a space in the parent directory large enough
 * to fit the new entry.
 * <p>
 * The passed directory entry will be updated with its position within the
 * parent directory.
 *
 * @param sfs the filesystem in which the directory resides
 * @param parent the directory into which to write the new entry
 * @param entry the entry to write
 * @return success or failure
 */
HIDDEN int sfs_dir_write_directory_entry(const struct sfs_filesystem* sfs,
        const struct directory_entry* parent, struct directory_entry* entry);

/**
 * Rewrite a directory entry with updated properties.
 * <p>
 * This function assumes the passed entry contains all the new values for its
 * properties and overwrites what is currently in the parent directory.
 * <p>
 * Do not use this function to change a file's name. This function does a
 * direct overwrite. Use change_file_name() to change a file's name.
 *
 * @param sfs the filesystem in which the directory entry resides
 * @param entry the directory entry to update
 * @return success or failure
 */
HIDDEN int sfs_dir_update_directory_entry(const struct sfs_filesystem* sfs,
        const struct directory_entry* entry);

/**
 * Rename a file.
 *
 * @param sfs the filesystem in which the file resides
 * @param file the file to rename
 * @param filename the file's new name
 * @return success or failure
 */
HIDDEN int sfs_dir_change_file_name(const struct sfs_filesystem* sfs,
        struct directory_entry* file, const char* filename);

#endif /* SFS_DIRECTORY_H */
