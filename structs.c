#include "structs.h"

void free_sfs(struct sfs_filesystem* sfs) {
    free(sfs);
}

void free_fat_entry(struct fat_entry* entry) {
    free(entry);
}

void free_fat_list(struct fat_list* list) {
    while (list) {
        struct fat_list* cur = list;
        list = list->next;
        free_fat_entry(cur->entry);
        free(cur);
    }
}

void free_directory_entry(struct directory_entry* entry) {
    free(entry->filename);
    free_directory_list(entry->contents);
    free(entry);
}

void free_directory_list(struct directory_list* list) {
    while (list) {
        struct directory_list* cur = list;
        list = list->next;
        free_directory_entry(cur->entry);
        free(cur);
    }
}

