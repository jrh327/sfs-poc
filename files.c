#include "files.h"

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

void write_directory_entry(FILE* fp, struct directory_entry* dir_entry) {
    fwrite(&(dir_entry->reserved), sizeof(dir_entry->reserved), 1, fp);
    fwrite(&(dir_entry->attributes), sizeof(dir_entry->attributes), 1, fp);

    uint8_t byte = (dir_entry->created_month << 4)
           | (dir_entry->created_day >> 1);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->created_day << 7) | (dir_entry->created_year);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->created_hour << 3) | (dir_entry->created_minute >> 3);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->created_minute << 5) | (dir_entry->created_second >> 1);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->created_second << 7) | (dir_entry->created_millisecond);
    fwrite(&byte, sizeof(byte), 1, fp);

    byte = (dir_entry->modified_month << 4) | (dir_entry->modified_day >> 1);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->modified_day << 7) | (dir_entry->modified_year);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->modified_hour << 3) | (dir_entry->modified_minute >> 3);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->modified_minute << 5)
            | (dir_entry->modified_second >> 1);
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = (dir_entry->modified_second << 7)
            | (dir_entry->modified_millisecond);
    fwrite(&byte, sizeof(byte), 1, fp);

    write_uint16(fp, dir_entry->table_number);
    write_uint16(fp, dir_entry->first_cluster);
    write_uint32(fp, dir_entry->file_length);

    fwrite(&(dir_entry->filename_entries),
            sizeof(dir_entry->filename_entries), 1, fp);
    
    for (size_t i = 0; i < 11; i++) {
        byte = dir_entry->filename[i];
        fwrite(&byte, sizeof(byte), 1, fp);
    }
}
