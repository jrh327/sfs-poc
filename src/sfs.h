/*
 * sfs.h
 *
 * Defines the public API for interacting with the SFS filesystem.
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#ifndef SFS_H
#define SFS_H

#include "sfs_includes.h"

/**
 * Create a new SFS filesystem with no partition offset.
 *
 * Note: this is the same as calling initialize_filesystem_partition()
 * with partition_offset set to 0.
 *
 * @param fd the file that will contain the SFS filesystem
 * @param fat_size the number of entries in the file allocation table
 * @param bytes_per_sector the number of bytes in a data sector
 * @param sectors_per_cluster the number of sectors in a data cluster
 * @return the newly created filesystem
 */
EXPORT struct sfs_filesystem* sfs_initialize_new_filesystem(int fd,
        uint16_t fat_size, uint16_t bytes_per_sector,
        uint8_t sectors_per_cluster);

/**
 * Create a new SFS filesystem partition at the given offset.
 *
 * @param fd the file that will contain the SFS filesystem
 * @param partition_offset how far into fd to start the filesystem
 * @param fat_size the number of entries in the file allocation table
 * @param bytes_per_sector the number of bytes in a data sector
 * @param sectors_per_cluster the number of sectors in a data cluster
 * @return the newly created filesystem
 */
EXPORT struct sfs_filesystem* sfs_initialize_filesystem_partition(int fd,
        uint64_t partition_offset, uint16_t fat_size, uint16_t bytes_per_sector,
        uint8_t sectors_per_cluster);

/**
 * Read the boot sector of a SFS filesystem.
 *
 * @param fd the file containing the SFS filesystem
 * @return the filesystem
 */
EXPORT struct sfs_filesystem* sfs_load_filesystem(int fd);

/**
 * Close the filesystem.
 *
 * @param sfs the filesystem
 * @return 0 if successful, or an error code
 */
EXPORT int sfs_close_filesystem(struct sfs_filesystem* sfs);

/**
 * Retrieve the directory entry for the root directory of the filesystem.
 *
 * @param sfs the filesystem to read from
 * @return the root directory's entry
 */
EXPORT struct directory_entry* sfs_get_root_directory(
        const struct sfs_filesystem*);

/**
 * Retrieve the names of the files within the given directory.
 *
 * @param sfs the filesystem to read from
 * @param dir the directory to scan for entries
 * @return 0 if successful, or an error code
 */
EXPORT struct directory_list* sfs_list_directory(
        const struct sfs_filesystem* sfs, const struct directory_entry* dir);

/**
 * Get information about an open file.
 *
 * @param sfs the filesystem in which the file resides
 * @param fd the file descriptor of the file
 * @return the file's information
 */
EXPORT struct file_stat* sfs_describe_file(const struct sfs_filesystem* sfs,
        const int fd);

/**
 * Get information about a file by filename.
 *
 * @param sfs the filesystem in which the file resides
 * @param dir the file descriptor of the file's parent directory
 * @param filename the name of the file to describe
 * @return the file's information
 */
EXPORT struct file_stat* sfs_describe_file_in_directory(
        const struct sfs_filesystem* sfs, const int dir, const char* filename);

/**
 * Create a new sub-directory in the given directory.
 *
 * @param sfs the filesystem to write to
 * @param parent the directory to create the sub-directory in
 * @param filename the name of the new sub-directory
 * @return the directory entry of the newly created sub-directory
 */
EXPORT struct directory_entry* sfs_create_directory(
        const struct sfs_filesystem* sfs, const struct directory_entry* parent,
        const char* filename);

/**
 * Create a new file with the given contents in the given directory.
 *
 * @param sfs the filesystem to write to
 * @param parent the directory to create the file in
 * @param filename the name of the new file
 * @param data the contents of the file
 * @param file_length the number of bytes in the file
 * @return the directory entry of the newly created file
 */
EXPORT struct directory_entry* sfs_create_file(const struct sfs_filesystem* sfs,
        struct directory_entry* parent, const char* filename,
        const uint8_t* data, const SFS_FILE_LENGTH file_length);

/**
 * Jump to an offset within the given file.
 * <p>
 * Use the following values for whence:
 * <ul>
 * <li>SEEK_SET - jump from the beginning of the file</li>
 * <li>SEEK_CUR - jump from the current offset</li>
 * <li>SEEK_END - jump from the end of the file</li>
 * </ul>
 *
 * @param sfs the filesystem in which the file resides
 * @param file the file to jump within
 * @param offset how far to jump
 * @param whence where in the file to jump from
 * @return the resulting position within the file
 */
EXPORT int sfs_seek_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file, SFS_FILE_OFFSET offset, int whence);

/**
 * Read the contents of a file.
 * <p>
 * This function reads from the current offset within the given file.
 * Use the seek_file() function to jump to a specific offset.
 *
 * @param sfs the filesystem to read from
 * @param file the file whose contents to read
 * @param buffer the array to put the read data into
 * @param length how many bytes to read
 * @return success or failure
 */
EXPORT int sfs_read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, uint8_t* buffer, uint64_t length);

/**
 * Delete a file.
 *
 * Its directory entry will be marked as deleted but not removed, and its
 * allocation table cluster chain will remain intact.
 *
 * @param sfs the filesystem to delete from
 * @param file the file to delete
 * @return 0 if successful, or an error code
 */
EXPORT int sfs_delete_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file);

/**
 * Restore a deleted file.
 *
 * @param sfs the filesystem to undelete from
 * @param file the file to undelete
 * @return 0 if successful, or an error code
 */
EXPORT int sfs_undelete_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file);

/**
 * Delete a file and remove all traces of it from the filesystem.
 *
 * Its directory entry will be removed and all following entries moved up. Its
 * allocation table cluster chain and its data clusters will be zeroed out.
 *
 * @param sfs the filesystem to delete from
 * @param file the file to delete
 * @return 0 if successful, or an error code
 */
EXPORT int sfs_hard_delete_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file);

/**
 * Change the name of a file.
 *
 * @param sfs the filesystem to write to
 * @param file the file to rename
 * @param filename the new filename
 * @return 0 if successful, or an error code
 */
EXPORT int sfs_rename_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file, const char* filename);

/**
 * Move a file to a new directory.
 *
 * @param sfs the filesystem to write to
 * @param file the file to move
 * @param new_parent the new directory to move the file to
 * @return 0 if successful, or an error code
 */
EXPORT int sfs_move_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file, const struct directory_entry* new_parent);

#endif /* SFS_H */
