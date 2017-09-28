/*
 * sfs_structs.c
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#include "sfs_includes.h"

HIDDEN void free_sfs(struct sfs_filesystem* sfs) {
    free(sfs);
}

HIDDEN void free_fat_entry(struct fat_entry* entry) {
    free(entry);
}

HIDDEN void free_fat_list(struct fat_list* list) {
    while (list) {
        struct fat_list* cur = list;
        list = list->next;
        free_fat_entry(cur->entry);
        free(cur);
    }
}

HIDDEN void free_directory_entry(struct directory_entry* entry) {
    free(entry->filename);
    if (entry->parent) {
        if (entry->key != entry->parent->key) {
            free(entry->key);
        }
    }

    /* no need to free first_cluster or current_cluster */
    /* they will always be set to an entry within clusters */
    free_fat_list(entry->clusters);

    free(entry);
}

HIDDEN void free_directory_listing(struct directory_listing* entry) {
    free(entry->filename);
    free(entry);
}

HIDDEN void free_directory_list(struct directory_list* list) {
    while (list) {
        struct directory_list* cur = list;
        list = list->next;
        free_directory_listing(cur->entry);
        free(cur);
    }
}
