#ifndef STRUCTS_H
#define STRUCTS_H

#define FAT_ENTRY_SIZE 4
#define DIR_ENTRY_SIZE 32

#define FAT_SIZE_SMALL 2048
#define FAT_SIZE_MEDIUM 4096
#define FAT_SIZE_LARGE 8192

#define BOOT_SECTOR_SIZE 512

#define END_CLUSTER_CHAIN 0xffff

/* forward declaration */
struct directory_list;

/**
 * Structure containing information stored in the boot sector of the
 * filesystem.
 */
struct sfs_filesystem {
    FILE* fp;
    uint64_t partition_offset;
    uint16_t entries_per_fat;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
};

/**
 * Structure containing information stored in a allocation table entry.
 */
struct fat_entry {
    uint16_t fat_number;
    uint16_t cluster_number;
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
    struct directory_list* contents;
    uint16_t dir_entry_number;
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
    uint16_t table_number;
    uint16_t first_cluster;
    uint32_t file_length;
    uint8_t filename_entries;
    char* filename;
};

/**
 * Linked list of directory entries.
 */
struct directory_list {
    struct directory_entry* entry;
    struct directory_list* next;
};

#endif /* STRUCTS_H */
