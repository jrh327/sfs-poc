/*
 * sfs_structs.h
 *
 * Defines structs and constants used by the filesystem.
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#ifndef SFS_STRUCTS_H
#define SFS_STRUCTS_H

#include "sfs_includes.h"

#define FAT_ENTRY_SIZE 4
#define DIR_ENTRY_SIZE 32

#define FAT_SIZE_SMALL 2048
#define FAT_SIZE_MEDIUM 4096
#define FAT_SIZE_LARGE 8192

#define MIN_BYTES_PER_SECTOR 0x0200
#define MAX_BYTES_PER_SECTOR 0x8000
#define MAX_BYTES_PER_CLUSTER 0x8000
#define MAX_SECTORS_PER_CLUSTER 0x0080;

#define BOOT_SECTOR_SIZE 512

#define END_CLUSTER_CHAIN 0xFFFF

#define SFS_FILENAME_MAX 256
#define SFS_FILENAME_MAX_BYTES 1024

#define SOFT_DELETED_DIR_ENTRY 0x80
#define HARD_DELETED_DIR_ENTRY 0xC0

#define ATTR_DIRECTORY 0x01
#define ATTR_READ_ONLY 0x02
#define ATTR_HIDDEN 0x04

typedef size_t SFS_FILE_LENGTH;
typedef off_t SFS_FILE_OFFSET;

/* forward declarations */
struct sfs_filesystem;
struct fat_entry;
struct fat_list;
struct directory_entry;
struct directory_list;
struct encryption_key;

/**
 * Structure containing information stored in a allocation table entry.
 */
struct fat_entry {
    uint16_t fat_number;
    uint16_t cluster_number;
};

/**
 * Represents a free FAT entry.
 */
HIDDEN static const struct fat_entry FREE_ENTRY = {
        .fat_number = 0,
        .cluster_number = 0
};

/**
 * Represents the final cluster in a file's FAT chain.
 */
HIDDEN static const struct fat_entry END_CHAIN = {
        .fat_number = END_CLUSTER_CHAIN,
        .cluster_number = END_CLUSTER_CHAIN
};

/**
 * Structure containing information stored in the boot sector of the
 * filesystem.
 */
struct sfs_filesystem {
    int fd;
    uint64_t partition_offset;
    uint16_t entries_per_fat;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    struct fat_entry* first_available_fat_entry;
    struct encryption_key* global_key;
};

/**
 * Linked list of FAT entries.
 */
struct fat_list {
    struct fat_entry* entry;
    struct fat_list* next;
};

/**
 * Structure describing a file entry in a directory.
 *
 * Directory entries are stored in 32 bytes. The layout of these bytes is:
 * 0x00: Reserved
 * 0x01: Attributes
 * 0x02: Created date (two bytes)
 *  - Month: four bits
 *  - Day: five bits
 *  - Year: seven bits (number of years since 2000, up to 2127)
 * 0x04: Created time (three bytes)
 *  - Hour: five bits
 *  - Minute: six bits
 *  - Second: six bits
 *  - Millisecond: seven bits (multiple of 10ms)
 * 0x07: Modified date (two bytes)
 * 0x09: Modified time (three bytes)
 * 0x0C: Number of allocation table of first cluster (two bytes)
 * 0x0E: Allocation table entry of first cluster (two bytes)
 * 0x10: File length (four bits)
 * 0x14: Number of following entries used for filename
 * 0x15: Filename (variable length, UTF-8 encoded)
 *
 * For a file whose name is longer than 11 bytes, the rest of the name shall
 * be stored in the directory entries immediately following.
 *
 * The location and dir_entry_number indicate where in the filesystem to find
 * this particular directory entry. They are generated when the directory
 * entry is read from its parent directory, and are not stored in the
 * filesystem itself.
 */
struct directory_entry {
    struct directory_entry* parent;
    struct encryption_key* key;
    uint16_t dir_entry_number;
    uint64_t current_offset;
    struct fat_list* clusters;
    struct fat_list* current_cluster;
    uint8_t reserved;
    uint8_t attributes;
    uint8_t created_month;
    uint8_t created_day;
    uint8_t created_year;
    uint8_t created_hour;
    uint8_t created_minute;
    uint8_t created_second;
    uint8_t created_millisecond;
    uint8_t modified_month;
    uint8_t modified_day;
    uint8_t modified_year;
    uint8_t modified_hour;
    uint8_t modified_minute;
    uint8_t modified_second;
    uint8_t modified_millisecond;
    struct fat_entry* first_cluster;
    uint32_t file_length;
    uint8_t filename_entries;
    char* filename;
};

struct file_stat {
    char* filename;
    uint8_t attributes;
    uint32_t file_length;
    time_t created;
    time_t modified;
};

/**
 * Structure containing a brief summary of a file in a directory.
 * <ul>
 * <li>filename contains the name of the file.</li>
 * <li>name_length contains the number of UTF-8 characters in the filename.
 * This can be up to 1024 bytes (4 bytes per character)</li>
 * <li>file_type describes the type of file (regular or directory)</li>
 */
struct directory_listing {
    char* filename;
    uint16_t name_length;
    uint8_t file_type;
};

/**
 * Linked list of directory entries.
 */
struct directory_list {
    struct directory_listing* entry;
    struct directory_list* next;
};

HIDDEN void free_sfs(struct sfs_filesystem* sfs);
HIDDEN void free_fat_entry(struct fat_entry* entry);
HIDDEN void free_fat_list(struct fat_list* list);
HIDDEN void free_directory_entry(struct directory_entry* entry);
HIDDEN void free_directory_listing(struct directory_listing* entry);
HIDDEN void free_directory_list(struct directory_list* list);

#endif /* SFS_STRUCTS_H */
