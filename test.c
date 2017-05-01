#include "sfs.h"
#include "files.h"
#include "util.h"
#include "encryption/aes.h"

const char* filename = "test.bin";

void delete_tmp_file() {
    int err = remove(filename);
    if (err != 0) {
        printf("Could not delete file\n");
    }
}

int test_writing_uint16() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }
    const uint16_t test_val = 0x1234;
    write_uint16(fp, test_val);
    fclose(fp);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error reading file\n");
        return (-1);
    }

    int ret = 0;
    uint8_t byte;
    fread(&byte, sizeof(byte), 1, fp);
    if (byte != 0x12) {
        printf("first byte - expected: %d, got %d\n", 0x12, byte);
        ret = -1;
    }

    fread(&byte, sizeof(byte), 1, fp);
    if (byte != 0x34) {
        printf("second byte - expected: %d, got %d\n", 0x34, byte);
        ret = -1;
    }
    fclose(fp);

    delete_tmp_file();

    return (ret);
}

int test_reading_uint16() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    uint8_t byte = 0x12;
    fwrite(&byte, sizeof(byte), 1, fp);
    byte = 0x34;
    fwrite(&byte, sizeof(byte), 1, fp);
    fclose(fp);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error reading file\n");
        return (-1);
    }

    int ret = 0;
    const uint16_t test_val = 0x1234;
    uint16_t read_val = read_uint16(fp);
    if (read_val != test_val) {
        printf("read value - expected: %d, got %d\n", test_val, read_val);
        ret = -1;
    }
    fclose(fp);

    delete_tmp_file();

    return (ret);
}

int test_reading_directory_entry() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    struct sfs_filesystem* sfs = initialize_new_filesystem(fp, 0, 512, 1);

    const uint8_t test_reserved = 0;
    const uint8_t test_attributes = 5;

    const uint8_t test_created_month = 3;
    const uint8_t test_created_day = 25;
    const uint8_t test_created_year = 17;
    const uint8_t test_created_hour = 23;
    const uint8_t test_created_minute = 54;
    const uint8_t test_created_second = 13;
    const uint8_t test_created_milli = 75;

    const uint8_t test_modified_month = 3;
    const uint8_t test_modified_day = 26;
    const uint8_t test_modified_year = 17;
    const uint8_t test_modified_hour = 0;
    const uint8_t test_modified_minute = 3;
    const uint8_t test_modified_second = 15;
    const uint8_t test_modified_milli = 23;

    const uint16_t test_table = 1;
    const uint16_t test_cluster = 3;
    const uint32_t test_file_length = 123456;
    const uint8_t test_entries = 0;
    const uint8_t test_filename[11] = {
            'f', 'i', 'l', 'e', 'n', 'a', 'm', 'e', 't', 'x', 't'
    };

    struct directory_entry* dir_entry = malloc(sizeof(struct directory_entry));
    dir_entry->reserved = test_reserved;
    dir_entry->attributes = test_attributes;
    dir_entry->created_month = test_created_month;
    dir_entry->created_day = test_created_day;
    dir_entry->created_year = test_created_year;
    dir_entry->created_hour = test_created_hour;
    dir_entry->created_minute = test_created_minute;
    dir_entry->created_second = test_created_second;
    dir_entry->created_millisecond = test_created_milli;
    dir_entry->modified_month = test_modified_month;
    dir_entry->modified_day = test_modified_day;
    dir_entry->modified_year = test_modified_year;
    dir_entry->modified_hour = test_modified_hour;
    dir_entry->modified_minute = test_modified_minute;
    dir_entry->modified_second = test_modified_second;
    dir_entry->modified_millisecond = test_modified_milli;
    dir_entry->table_number = test_table;
    dir_entry->first_cluster = test_cluster;
    dir_entry->file_length = test_file_length;
    dir_entry->filename_entries = test_entries;

    dir_entry->filename = malloc(sizeof(test_filename));
    for (size_t i = 0; i < 11; i++) {
        dir_entry->filename[i] = test_filename[i];
    }

    dir_entry->parent = get_root_directory(sfs);
    write_directory_entry(sfs, dir_entry);

    free(dir_entry->filename);
    free(dir_entry);
    fclose(sfs->fp);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error reading file\n");
        return (-1);
    }

    sfs = load_filesystem(fp);
    sfs->fp = fp;
    struct directory_entry* root = get_root_directory(sfs);
    get_directory_entries(sfs, root);
    dir_entry = root->contents->entry; /* first entry in the root */

    fclose(fp);

    //delete_tmp_file();

    int ret = 0;
    if (dir_entry->reserved != test_reserved) {
        printf("reserved - expected %d, got %d\n", test_reserved,
                dir_entry->reserved);
        ret = -1;
    }

    if (dir_entry->attributes != test_attributes) {
        printf("attributes - expected %d, got %d\n", test_attributes,
                dir_entry->attributes);
        ret = -1;
    }

    if (dir_entry->created_month != test_created_month) {
        printf("created_month - expected %d, got %d\n", test_created_month,
                dir_entry->created_month);
        ret = -1;
    }

    if (dir_entry->created_day != test_created_day) {
        printf("created_day - expected %d, got %d\n", test_created_day,
                dir_entry->created_day);
        ret = -1;
    }

    if (dir_entry->created_year != test_created_year) {
        printf("created_year - expected %d, got %d\n", test_created_year,
                dir_entry->created_year);
        ret = -1;
    }

    if (dir_entry->created_hour != test_created_hour) {
        printf("created_hour - expected %d, got %d\n", test_created_hour,
                dir_entry->created_hour);
        ret = -1;
    }

    if (dir_entry->created_minute != test_created_minute) {
        printf("created_minute - expected %d, got %d\n", test_created_minute,
                dir_entry->created_minute);
        ret = -1;
    }

    if (dir_entry->created_second != test_created_second) {
        printf("created_second - expected %d, got %d\n", test_created_second,
                dir_entry->created_second);
        ret = -1;
    }

    if (dir_entry->created_millisecond != test_created_milli) {
        printf("created_millisecond - expected %d, got %d\n",
                test_created_milli, dir_entry->created_millisecond);
        ret = -1;
    }

    if (dir_entry->modified_month != test_modified_month) {
        printf("modified_month - expected %d, got %d\n", test_modified_month,
                dir_entry->modified_month);
        ret = -1;
    }

    if (dir_entry->modified_day != test_modified_day) {
        printf("modified_day - expected %d, got %d\n", test_modified_day,
                dir_entry->modified_day);
        ret = -1;
    }

    if (dir_entry->modified_year != test_modified_year) {
        printf("modified_year - expected %d, got %d\n", test_modified_year,
                dir_entry->modified_year);
        ret = -1;
    }

    if (dir_entry->modified_hour != test_modified_hour) {
        printf("modified_hour - expected %d, got %d\n", test_modified_hour,
                dir_entry->modified_hour);
        ret = -1;
    }

    if (dir_entry->modified_minute != test_modified_minute) {
        printf("modified_minute - expected %d, got %d\n", test_modified_minute,
                dir_entry->modified_minute);
        ret = -1;
    }

    if (dir_entry->modified_second != test_modified_second) {
        printf("modified_second - expected %d, got %d\n", test_modified_second,
                dir_entry->modified_second);
        ret = -1;
    }

    if (dir_entry->modified_millisecond != test_modified_milli) {
        printf("modified_millisecond - expected %d, got %d\n",
                test_modified_milli, dir_entry->modified_millisecond);
        ret = -1;
    }

    if (dir_entry->table_number != test_table) {
        printf("table_number - expected %d, got %d\n", test_table,
                dir_entry->table_number);
        ret = -1;
    }

    if (dir_entry->first_cluster != test_cluster) {
        printf("first_cluster - expected %d, got %d\n", test_cluster,
                dir_entry->first_cluster);
        ret = -1;
    }

    if (dir_entry->file_length != test_file_length) {
        printf("file_length - expected %d, got %d\n", test_file_length,
                dir_entry->file_length);
        ret = -1;
    }

    if (dir_entry->filename_entries != test_entries) {
        printf("filename_entries - expected %d, got %d\n", test_entries,
                dir_entry->filename_entries);
        ret = -1;
    }

    for (size_t i = 0; i < 11; i++) {
        if (dir_entry->filename[i] != test_filename[i]) {
            printf("filename[%zu] - expected %c, got %c\n", i, test_filename[i],
                    dir_entry->filename[i]);
            ret = -1;
        }
    }

    return (ret);
}

int test_read_dir_entry_short_filename() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    struct sfs_filesystem* sfs = initialize_new_filesystem(fp, 0, 512, 1);
    const uint8_t test_filename[9] = {
            'f', 'i', 'l', 'e', '.', 't', 'x', 't', '\0'
    };
    struct directory_entry* dir_entry = malloc(sizeof(struct directory_entry));
    dir_entry->filename_entries = 0;
    dir_entry->filename = malloc(sizeof(test_filename));
    for (size_t i = 0; i < 11; i++) {
        dir_entry->filename[i] = test_filename[i];
    }

    dir_entry->parent = get_root_directory(sfs);

    write_directory_entry(sfs, dir_entry);

    free(dir_entry->filename);
    free(dir_entry);
    fclose(fp);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error reading file\n");
        return (-1);
    }

    sfs = load_filesystem(fp);
    sfs->fp = fp;
    struct directory_entry* root = get_root_directory(sfs);
    get_directory_entries(sfs, root);
    dir_entry = root->contents->entry; /* first entry in the root */

    fclose(fp);

    delete_tmp_file();

    int ret = 0;
    for (size_t i = 0; i < 9; i++) {
        if (dir_entry->filename[i] != test_filename[i]) {
            printf("filename[%zu] - expected %c, got %c\n", i, test_filename[i],
                    dir_entry->filename[i]);
            ret = -1;
        }
    }
    return ret;
}

int test_read_dir_entry_long_filename() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    struct sfs_filesystem* sfs = initialize_new_filesystem(fp, 0, 512, 1);
    const char* test_filename = "filenametxtfilenametxtfilenametxtfilenametxtfilenametxtfilenametxt";
    struct directory_entry* dir_entry = malloc(sizeof(struct directory_entry));
    dir_entry->filename_entries = 2;
    size_t len = strlen(test_filename);
    dir_entry->filename = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        dir_entry->filename[i] = test_filename[i];
    }
    dir_entry->filename[len] = 0;

    dir_entry->parent = get_root_directory(sfs);

    write_directory_entry(sfs, dir_entry);

    free(dir_entry->filename);
    free(dir_entry);
    fclose(fp);

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("error reading file\n");
        return (-1);
    }

    sfs->fp = fp;
    struct directory_entry* root = get_root_directory(sfs);
    get_directory_entries(sfs, root);
    dir_entry = root->contents->entry; /* first entry in the root */

    int ret = 0;

    /* make sure reserved bytes are set correctly */
    fseek(fp, BOOT_SECTOR_SIZE + sfs->entries_per_fat * FAT_ENTRY_SIZE, SEEK_SET);
    for (size_t i = 0; i < 3; i++) {
        uint8_t entry[DIR_ENTRY_SIZE] = { 0 };
        fread(&entry, DIR_ENTRY_SIZE, 1, fp);
        if (entry[0] != i) {
            printf("entry %zu - expected reserved byte to be %zu, got %d\n",
                    i, i, entry[0]);
            ret = -1;
        }
    }
    fclose(fp);

    //delete_tmp_file();

    char* filename = dir_entry->filename;
    for (size_t i = 0; i < len; i++) {
        if (*filename != test_filename[i]) {
            printf("filename[%zu] - expected %c, got %c\n", i, test_filename[i],
                    *filename);
            ret = -1;
        }
        filename++;
    }
    return ret;
}

int test_new_bootsector_constraints() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    uint16_t init_fat_size = FAT_SIZE_MEDIUM;
    uint16_t init_bytes_per_sector = 512;
    uint8_t init_sectors_per_cluster = 64;
    uint16_t expected_fat_size = FAT_SIZE_MEDIUM;
    uint16_t expected_bytes_per_sector = 512;
    uint8_t expected_sectors_per_cluster = 64;
    struct sfs_filesystem* sfs = initialize_filesystem_partition(fp, 0,
            init_fat_size, init_bytes_per_sector, init_sectors_per_cluster);
    int ret = 0;
    if (sfs->entries_per_fat != expected_fat_size) {
        printf("entries_per_fat 1 - expected %d, got %d\n", expected_fat_size,
                sfs->entries_per_fat);
        ret = -1;
    }

    if (sfs->bytes_per_sector != expected_bytes_per_sector) {
        printf("bytes_per_sector 1 - expected %d, got %d\n",
                expected_bytes_per_sector, sfs->bytes_per_sector);
        ret = -1;
    }

    if (sfs->sectors_per_cluster != expected_sectors_per_cluster) {
        printf("sectors_per_cluster 1 - expected %d, got %d\n",
                expected_sectors_per_cluster, sfs->sectors_per_cluster);
        ret = -1;
    }

    fclose(fp);

    delete_tmp_file();

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    init_fat_size = FAT_SIZE_MEDIUM - 1;
    init_bytes_per_sector = 500;
    init_sectors_per_cluster = 128;
    expected_fat_size = FAT_SIZE_MEDIUM; /* default */
    expected_bytes_per_sector = 512; /* minimum */
    expected_sectors_per_cluster = 64; /* 32K byte max cluster size */
    sfs = initialize_filesystem_partition(fp, 0, init_fat_size,
            init_bytes_per_sector, init_sectors_per_cluster);
    if (sfs->entries_per_fat != expected_fat_size) {
        printf("entries_per_fat 2 - expected %d, got %d\n", expected_fat_size,
                sfs->entries_per_fat);
        ret = -1;
    }

    if (sfs->bytes_per_sector != expected_bytes_per_sector) {
        printf("bytes_per_sector 2 - expected %d, got %d\n",
                expected_bytes_per_sector, sfs->bytes_per_sector);
        ret = -1;
    }

    if (sfs->sectors_per_cluster != expected_sectors_per_cluster) {
        printf("sectors_per_cluster 2 - expected %d, got %d\n",
                expected_sectors_per_cluster, sfs->sectors_per_cluster);
        ret = -1;
    }

    fclose(fp);

    delete_tmp_file();

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    init_fat_size = FAT_SIZE_MEDIUM;
    init_bytes_per_sector = 544;
    init_sectors_per_cluster = 20;
    expected_fat_size = FAT_SIZE_MEDIUM; /* default */
    expected_bytes_per_sector = 512; /* uses MSB if not power of 2 */
    expected_sectors_per_cluster = 16; /* uses MSB if not power of 2 */
    sfs = initialize_filesystem_partition(fp, 0, init_fat_size,
            init_bytes_per_sector, init_sectors_per_cluster);
    if (sfs->entries_per_fat != expected_fat_size) {
        printf("entries_per_fat 3 - expected %d, got %d\n", expected_fat_size,
                sfs->entries_per_fat);
        ret = -1;
    }

    if (sfs->bytes_per_sector != expected_bytes_per_sector) {
        printf("bytes_per_sector 3 - expected %d, got %d\n",
                expected_bytes_per_sector, sfs->bytes_per_sector);
        ret = -1;
    }

    if (sfs->sectors_per_cluster != expected_sectors_per_cluster) {
        printf("sectors_per_cluster 3 - expected %d, got %d\n",
                expected_sectors_per_cluster, sfs->sectors_per_cluster);
        ret = -1;
    }

    fclose(fp);

    delete_tmp_file();

    return (ret);
}

int test_create_file() {
    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("error creating file\n");
        return (-1);
    }

    const uint16_t fat_size = FAT_SIZE_SMALL;
    const uint16_t bytes_per_sector = 512;
    const uint16_t sectors_per_cluster = 1;
    struct sfs_filesystem* sfs = initialize_new_filesystem(fp, fat_size,
            bytes_per_sector, sectors_per_cluster);
    struct directory_entry* root = get_root_directory(sfs);

    const char* filename = "test.txt";
    uint64_t file_length = 52;
    uint8_t* data = malloc(file_length);
    uint8_t* ptr_data = data;
    for (size_t i = 0; i < 26; i++) {
        *ptr_data = (char)('a' + i);
        ptr_data++;
    }
    for (size_t i = 0; i < 26; i++) {
        *ptr_data = (char)('A' + i);
        ptr_data++;
    }

    struct directory_entry* file = create_file(sfs, root, "test.txt", data,
            file_length);

    free(data);

    int ret = 0;
    if (strcmp(file->filename, filename)) {
        printf("filename - expected %s, got %s\n",
            filename, file->filename);
        ret = -1;
    }
    if (file->table_number != 0) {
        printf("table number - expected %d, got %d\n",
                0, file->table_number);
        ret = -1;
    }
    if (file->first_cluster != 1) {
        printf("first cluster - expected %d, got %d\n",
                1, file->first_cluster);
        ret = -1;
    }
    if (file->file_length != 52) {
        printf("file length - expected %d, got %d\n",
                52, file->file_length);
        ret = -1;
    }

    uint64_t file_start = BOOT_SECTOR_SIZE
            + sfs->entries_per_fat * FAT_ENTRY_SIZE
            + sfs->bytes_per_sector * sfs->sectors_per_cluster;
    fseek(fp, file_start, SEEK_SET);
    data = malloc(file_length);
    fread(data, file_length, 1, fp);
    ptr_data = data;
    for (size_t i = 0; i < 26; i++) {
        if (*ptr_data != (char)('a' + i)) {
            printf("file data - expected %c, got %c\n",
                    (char)('a' + i), *ptr_data);
            ret = -1;
        }
        ptr_data++;
    }
    for (size_t i = 0; i < 26; i++) {
        if (*ptr_data != (char)('A' + i)) {
            printf("file data - expected %c, got %c\n",
                    (char)('A' + i), *ptr_data);
            ret = -1;
        }
        ptr_data++;
    }

    delete_tmp_file();

    return (ret);
}

void run_test(char* test_name, int (*test)()) {
    printf("--------------------\n");
    int result = test();
    if (result != 0) {
        printf("%s() failed\n", test_name);
    } else {
        printf("%s() passed\n", test_name);
    }
    printf("--------------------\n\n");
}

int main() {
    run_test("test_writing_uint16", test_writing_uint16);
    run_test("test_reading_uint16", test_reading_uint16);
    run_test("test_reading_directory_entry", test_reading_directory_entry);
    run_test("test_new_bootsector_constraints",
            test_new_bootsector_constraints);
    run_test("test_read_dir_entry_short_filename",
            test_read_dir_entry_short_filename);
    run_test("test_read_dir_entry_long_filename",
            test_read_dir_entry_long_filename);
    run_test("test_create_file", test_create_file);

    /* test_aes(0, NULL); */
}
