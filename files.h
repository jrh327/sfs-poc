#ifndef FILES_H
#define FILES_H

#include "includes.h"
#include "structs.h"
#include "util.h"

/**
 * Create a directory entry with the given properties.
 *
 * @param parent the parent directory of the new entry
 * @param filename the name of the new entry
 * @param file_length the length of the file represented by the new entry
 * @return the newly created directory entry
 */
struct directory_entry* create_directory_entry(struct directory_entry* parent,
        const char* filename, const uint64_t file_length);

/**
 * Read a directory entry from the filesystem.
 * 
 * @param sfs the filesystem to read from
 * @return the directory entry
 */
struct directory_entry* read_directory_entry(const struct sfs_filesystem* sfs,
        struct directory_entry* parent);

/**
 * Write a directory entry to the filesystem.
 * 
 * @param sfs the filesystem to write to
 * @param dir_entry the directory entry to write
 */
void write_directory_entry(const struct sfs_filesystem* sfs,
        struct directory_entry* dir_entry);

/**
 * Change a file's name.
 *
 * @param sfs the filesystem to write to
 * @param file the file whose name is being changed
 * @param filename the new filename
 */
void change_file_name(const struct sfs_filesystem* sfs,
        struct directory_entry* file, const char* filename);

/**
 * Retrieve the directory entry for the root directory of the filesystem.
 *
 * @param sfs the filesystem to read from
 * @return the root directory's entry
 */
struct directory_entry* get_root_directory_entry(const struct sfs_filesystem* sfs);

/**
 * Retrieve the directory entries within the given directory.
 *
 * The list of directory entries is stored in dir's contents attribute.
 *
 * @param sfs the filesystem to read from
 * @param parent the directory to scan for entries
 */
int get_directory_entries(const struct sfs_filesystem* sfs,
        struct directory_entry* parent);

/**
 * Get the data from a cluster.
 * 
 * @param sfs the filesystem to read from
 * @param entry the location of the cluster to retrieve
 * @return the data in the cluster
 */
uint8_t* read_file_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry);

/**
 * Read data from the given clusters.
 *
 * @param sfs the filesystem to read from
 * @param clusters the clusters to read from
 * @param file_length length of the data to read
 */
uint8_t* read_file_clusters(const struct sfs_filesystem* sfs,
        struct fat_list* clusters, const uint64_t file_length);

/**
 * Write a cluster to the filesystem.
 * 
 * @param sfs the filesystem to write to
 * @param entry the location to write to
 * @param cluster the bytes of the cluster
 */
void write_file_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry entry, const uint8_t* cluster);

/**
 * Write data to the given clusters.
 *
 * @param sfs the filesystem to write to
 * @param clusters the clusters to write to
 * @param data the data to write
 * @param file_length length of the data
 */
void write_file_clusters(const struct sfs_filesystem* sfs,
        struct fat_list* clusters, const uint8_t* data,
        const uint64_t file_length);

#endif /* FILES_H */
