#include "files.h"

struct directory_entry read_directory_entry(FILE* fp) {
    uint8_t entry[32] = { 0 };
    fread(&entry, sizeof(entry), 1, fp);

    uint8_t created_month = (entry[2] & 0xf0) >> 4;
    uint8_t created_day = ((entry[2] & 0x0f) << 1)
            | ((entry[3] & 0x80) >> 7);
    uint8_t created_year = entry[3] & 0x7f;
    uint8_t created_hour = (entry[4] & 0xf8) >> 3;
    uint8_t created_minute = ((entry[4] & 0x7) << 3)
            | ((entry[5] & 0xe0) >> 5);
    uint8_t created_second = ((entry[5] & 0x1f) << 1)
            | ((entry[6] & 0x80) >> 7);
    uint8_t created_millisecond = entry[6] & 0x7f;

    uint8_t modified_month = (entry[7] & 0xf0) >> 4;
    uint8_t modified_day = ((entry[7] & 0x0f) << 1)
            | ((entry[8] & 0x80) >> 7);
    uint8_t modified_year = entry[8] & 0x7f;
    uint8_t modified_hour = (entry[9] & 0xf8) >> 3;
    uint8_t modified_minute = ((entry[9] & 0x7) << 3)
            | ((entry[10] & 0xe0) >> 5);
    uint8_t modified_second = ((entry[10] & 0x1f) << 1)
            | ((entry[11] & 0x80) >> 7);
    uint8_t modified_millisecond = entry[11] & 0x7f;

    struct directory_entry dir_entry = {
        .reserved = entry[0],
        .attributes = entry[1],

        .created_month = created_month,
        .created_day = created_day,
        .created_year = created_year,
        .created_hour = created_hour,
        .created_minute = created_minute,
        .created_second = created_second,
        .created_millisecond = created_millisecond,

        .modified_month = modified_month,
        .modified_day = modified_day,
        .modified_year = modified_year,
        .modified_hour = modified_hour,
        .modified_minute = modified_minute,
        .modified_second = modified_second,
        .modified_millisecond = modified_millisecond,

        .table_number = get_uint16(entry, 12),
        .first_cluster = get_uint16(entry, 14),
        .file_length = get_uint32(entry, 16)
    };
    dir_entry.filename_entries = entry[20];

    for (size_t i = 0; i < 11; i++) {
        dir_entry.filename[i] = entry[21 + i];
    }

    return dir_entry;
}

void write_directory_entry(FILE* fp, struct directory_entry dir_entry) {
    uint8_t entry[32] = { 0 };

    entry[0] = dir_entry.reserved;
    entry[1] = dir_entry.attributes;

    entry[2] = (dir_entry.created_month << 4) | (dir_entry.created_day >> 1);
    entry[3] = (dir_entry.created_day << 7) | (dir_entry.created_year);
    entry[4] = (dir_entry.created_hour << 3)
            | (dir_entry.created_minute >> 3);
    entry[5] = (dir_entry.created_minute << 5)
            | (dir_entry.created_second >> 1);
    entry[6] = (dir_entry.created_second << 7)
            | (dir_entry.created_millisecond);

    entry[7] = (dir_entry.modified_month << 4) | (dir_entry.modified_day >> 1);
    entry[8] = (dir_entry.modified_day << 7) | (dir_entry.modified_year);
    entry[9] = (dir_entry.modified_hour << 3) | (dir_entry.modified_minute >> 3);
    entry[10] = (dir_entry.modified_minute << 5)
            | (dir_entry.modified_second >> 1);
    entry[11] = (dir_entry.modified_second << 7)
            | (dir_entry.modified_millisecond);

    put_uint16(entry, dir_entry.table_number, 12);
    put_uint16(entry, dir_entry.first_cluster, 14);
    put_uint32(entry, dir_entry.file_length, 16);

    entry[20] = dir_entry.filename_entries;
    
    for (size_t i = 0; i < 11; i++) {
        entry[21 + i] = dir_entry.filename[i];
    }

    fwrite(&entry, sizeof(entry), 1, fp);
}
