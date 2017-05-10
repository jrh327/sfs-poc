#ifndef SFS_H
#define SFS_H

#include "includes.h"
#include "structs.h"

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
struct sfs_filesystem* initialize_new_filesystem(int fd, uint16_t fat_size,
        uint16_t bytes_per_sector, uint8_t sectors_per_cluster);

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
struct sfs_filesystem* initialize_filesystem_partition(int fd,
        uint64_t partition_offset, uint16_t fat_size, uint16_t bytes_per_sector,
        uint8_t sectors_per_cluster);

/**
 * Read the boot sector of a SFS filesystem.
 * 
 * @param fd the file containing the SFS filesystem
 * @return the filesystem
 */
struct sfs_filesystem* load_filesystem(int fd);

/**
 * Close the filesystem.
 * 
 * @param sfs the filesystem
 * @return 0 if successful, or an error code
 */
int close_filesystem(struct sfs_filesystem* sfs);

/**
 * Retrieve the directory entry for the root directory of the filesystem.
 *
 * @param sfs the filesystem to read from
 * @return the root directory's entry
 */
struct directory_entry* get_root_directory(const struct sfs_filesystem*);

/**
 * Retrieve the directory entries within the given directory.
 *
 * The list of directory entries is stored in dir's contents attribute.
 *
 * @param sfs the filesystem to read from
 * @param dir the directory to scan for entries
 * @return 0 if successful, or an error code
 */
int list_directory(const struct sfs_filesystem* sfs,
        struct directory_entry* dir);

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
struct directory_entry* create_file(const struct sfs_filesystem* sfs,
        struct directory_entry* parent, const char* filename,
        const uint8_t* data, const uint64_t file_length);

/**
 * Read the contents of a file.
 *
 * @param sfs the filesystem to read from
 * @param file the file whose contents to read
 * @return the contents of the file. size will equal file->file_length
 */
uint8_t* read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file);

/**
 * Rewrite a portion of a file.
 *
 * @param sfs the filesystem to write to
 * @param file the file to update
 * @param data the new contents, starting with the first change
 * @param offset the location of the first change within the file
 * @return 0 if successful, or an error code
 */
int update_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, const uint8_t* data, int offset);

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
int delete_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file);

/**
 * Restore a deleted file.
 *
 * @param sfs the filesystem to undelete from
 * @param file the file to undelete
 * @return 0 if successful, or an error code
 */
int undelete_file(const struct sfs_filesystem* sfs,
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
int hard_delete_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file);

/**
 * Change the name of a file.
 *
 * @param sfs the filesystem to write to
 * @param file the file to rename
 * @param filename the new filename
 * @return 0 if successful, or an error code
 */
int rename_file(const struct sfs_filesystem* sfs, struct directory_entry* file,
        const char* filename);

/**
 * Move a file to a new directory.
 *
 * @param sfs the filesystem to write to
 * @param file the file to move
 * @param new_parent the new directory to move the file to
 * @return 0 if successful, or an error code
 */
int move_file(const struct sfs_filesystem* sfs, struct directory_entry* file,
        const struct directory_entry* new_parent);

#endif /* SFS_H */
