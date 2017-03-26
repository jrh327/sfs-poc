#ifndef SFS_H
#define SFS_H 1

#include "util.h"

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
 * be stored in the following entries
 */
struct directory_entry {
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
    uint8_t filename[11];
};

struct directory_entry* read_directory_entry(FILE* fp) {
    uint8_t entry[32];
    fread(&entry, sizeof(entry), 1, fp);

    struct directory_entry* dir_entry = malloc(sizeof(struct directory_entry));
    dir_entry->reserved = entry[0];
    dir_entry->attributes = entry[1];

    dir_entry->created_month = (entry[2] & 0xf0) >> 4;
    dir_entry->created_day = ((entry[2] & 0x0f) << 1)
            | ((entry[3] & 0x80) >> 7);
    dir_entry->created_year = entry[3] & 0x7f;
    dir_entry->created_hour = (entry[4] & 0xf8) >> 3;
    dir_entry->created_minute = ((entry[4] & 0x7) << 3)
            | ((entry[5] & 0xe0) >> 5);
    dir_entry->created_second = ((entry[5] & 0x1f) << 1)
            | ((entry[6] & 0x80) >> 7);
    dir_entry->created_millisecond = entry[6] & 0x7f;

    dir_entry->modified_month = (entry[7] & 0xf0) >> 4;
    dir_entry->modified_day = ((entry[7] & 0x0f) << 1)
            | ((entry[8] & 0x80) >> 7);
    dir_entry->modified_year = entry[8] & 0x7f;
    dir_entry->modified_hour = (entry[9] & 0xf8) >> 3;
    dir_entry->modified_minute = ((entry[9] & 0x7) << 3)
            | ((entry[10] & 0xe0) >> 5);
    dir_entry->modified_second = ((entry[10] & 0x1f) << 1)
            | ((entry[11] & 0x80) >> 7);
    dir_entry->modified_millisecond = entry[11] & 0x7f;

    dir_entry->table_number = (entry[12] << 8) | entry[13];
    dir_entry->first_cluster = (entry[14] << 8) | entry[15];
    dir_entry->file_length = (entry[16] << 24) | (entry[17] << 16)
            | (entry[18] << 8) | entry[19];

    dir_entry->filename_entries = entry[20];

    for (size_t i = 0; i < 11; i++) {
        dir_entry->filename[i] = entry[21 + i];
    }

    return dir_entry;
}

#endif /* SFS_H */
