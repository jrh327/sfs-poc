#ifndef ALLOCATION_H_
#define ALLOCATION_H_

#include "includes.h"
#include "structs.h"

/**
 * Jump to a particular FAT in the filesystem.
 *
 * The FAT is generated if it is past the current end of the filesystem.
 *
 * @param sfs the filesystem the FAT is in
 * @param fat_number the number of the FAT to jump to
 */
void jump_to_fat(const struct sfs_filesystem* sfs, uint16_t fat_number);

/**
 * Jump to a particular data cluster in the filesystem.
 *
 * The cluster is generated if it is past the current end of the filesystem.
 *
 * @param sfs the filesystem the cluster is in
 * @param entry a FAT entry representing the cluster to jump to
 */
void jump_to_cluster(const struct sfs_filesystem* sfs, struct fat_entry entry);

/**
 * Find the first empty location after the given FAT entry.
 *
 * @param sfs the filesystem in which to search
 * @param start the FAT entry at which to start searching
 * @return the next available FAT entry
 */
struct fat_entry find_next_avail_fat_entry(const struct sfs_filesystem* sfs,
        struct fat_entry start);

/**
 * Get an entry from a file allocation table.
 *
 * @param sfs the filesystem to read from
 * @param entry the location of the entry to retrieve
 * @return the value in the entry
 */
struct fat_entry get_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry entry);

/**
 * Put an entry into a file allocation table.
 *
 * @param sfs the filesystem to write to
 * @param location the location of the entry to write
 * @param entry the entry to write
 */
void put_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry location, const struct fat_entry entry);

/**
 * Allocate a cluster at the end of a file.
 *
 * @param sfs the filesystem to write to
 * @param list the FAT list of the file to extend
 * @return the FAT entry of the newly allocated cluster
 */
struct fat_entry allocate_cluster(const struct sfs_filesystem* sfs,
        struct fat_list* list);

/**
 * Generate a list of available clusters for a new file of given length.
 *
 * @param sfs the filesystem in which to allocate
 * @param file_len the number of bytes to allocate
 * @return ordered list of FAT entries corresponding to the allocated clusters
 */
struct fat_list* allocate_file(const struct sfs_filesystem* sfs,
        uint64_t file_len);

/**
 * Get the list of clusters occupied by a given file.
 *
 * @param sfs the filesystem in which the file resides
 * @param file the file whose clusters are to be retrieved
 * @return ordered list of FAT entries corresponding to file's clusters
 */
struct fat_list* get_file_clusters(const struct sfs_filesystem* sfs,
        const struct directory_entry* file);

#endif /* ALLOCATION_H_ */
