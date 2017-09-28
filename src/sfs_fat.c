/*
 * sfs_fat.c
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#include "sfs_includes.h"

static int sfs_fat_jump_to_fat(const struct sfs_filesystem* sfs, int fat_number) {
    uint64_t begin_data = BOOT_SECTOR_SIZE;
    uint64_t sizeof_fat = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    uint64_t sizeof_data_block = sfs->entries_per_fat /* clusters in block */
            * sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint64_t location = begin_data
            + fat_number * (sizeof_fat + sizeof_data_block);

    int fd = sfs->fd;

    /* check if going past the current end of the filesystem */
    sfs_util_seek_in_medium(fd, 0, SEEK_END);
    uint64_t size = sfs_util_tell_file(fd);
    sfs_util_seek_in_medium(fd, location, SEEK_SET);

    /* generate the FAT if we went past the end of the existing filesystem */
    if (size < location) {
        sfs_io_write_new_fat(sfs);
    }

    off_t ret = sfs_util_seek_in_medium(fd, location, SEEK_SET);
    if (ret == -1) {
        return (-1);
    }

    return (0);
}

/* @param entry the location of the FAT entry to jump to */
HIDDEN int sfs_fat_jump_to_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry) {
	sfs_fat_jump_to_fat(sfs, entry->fat_number);
    sfs_util_seek_in_medium(sfs->fd, entry->cluster_number * FAT_ENTRY_SIZE,
            SEEK_CUR);
    return (0);
}

/* @param entry the location of the entry to read */
/* @return the value in the FAT entry */
HIDDEN struct fat_entry* sfs_fat_read_fat_entry(
        const struct sfs_filesystem* sfs, const struct fat_entry* location) {
    sfs_fat_jump_to_fat_entry(sfs, location);
    return (sfs_io_read_fat_entry(sfs));
}

/* @param fat_pos the location of the entry to write */
/* @param entry the value to write into the entry */
HIDDEN int sfs_fat_write_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry* location, const struct fat_entry* entry) {
    sfs_fat_jump_to_fat_entry(sfs, location);
    int ret = sfs_io_write_fat_entry(sfs, entry);
    return (ret);
}

/* @param entry the location of the entry to mark */
HIDDEN int sfs_fat_mark_as_free(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry) {
    sfs_fat_write_fat_entry(sfs, entry, &FREE_ENTRY);
    struct fat_entry* avail = sfs->first_available_fat_entry;
    if (entry->fat_number < avail->fat_number) {
        avail->fat_number = entry->fat_number;
        avail->cluster_number = entry->cluster_number;
    } else if (entry->fat_number == avail->fat_number) {
        if (entry->cluster_number < avail->cluster_number) {
            avail->fat_number = entry->fat_number;
            avail->cluster_number = entry->cluster_number;
        }
    }
    return (0);
}

/* @param entry the location of the entry to mark */
HIDDEN int sfs_fat_mark_as_end_of_chain(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry) {
    sfs_fat_write_fat_entry(sfs, entry, &END_CHAIN);
    return (0);
}

/* @param entry the location of the new end-of-the-chain entry */
HIDDEN int sfs_fat_truncate_fat_chain(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry) {
    struct fat_list* chain = allocate(sizeof(struct fat_list));
    struct fat_entry* new_entry = allocate(FAT_ENTRY_SIZE);
    new_entry->fat_number = entry->fat_number;
    new_entry->cluster_number = entry->cluster_number;
    chain->entry = new_entry; /* location of the new end of the chain */
    chain->next = NULL;

    struct fat_entry* next_entry = chain->entry;
    int end_of_chain = 0;
    while (!end_of_chain) {
        next_entry = sfs_fat_read_fat_entry(sfs, next_entry);
        struct fat_list* next_chain = allocate(sizeof(struct fat_list));
        next_chain->entry = next_entry; /* location of next entry in the chain */
        next_chain->next = chain;
        chain = next_chain; /* stick next entry in the front of the list */
        /* want to start with the end of the chain and work backwards to the
         * fat_entry we want as the new end of the chain, so in case there is
         * a crash, we won't be leaving the entries at the end orphaned and
         * unrecoverable
         */
        end_of_chain = (next_entry->fat_number == END_CLUSTER_CHAIN
                && next_entry->cluster_number == END_CLUSTER_CHAIN);
    }

    /* final entry added was END_CHAIN. need to point to its location */
    struct fat_list* final_chain = chain;
    chain = chain->next;
    while (chain->next != NULL) {
        sfs_fat_mark_as_free(sfs, chain->entry);
        chain = chain->next;
    }

    sfs_fat_mark_as_end_of_chain(sfs, chain->entry);

    free_fat_list(final_chain);

    return (0);
}

/* @param entry the location of the entry at which to start searching */
/* @return the location of an empty FAT entry */
HIDDEN struct fat_entry* sfs_fat_find_next_empty_fat_entry(
        const struct sfs_filesystem* sfs, const struct fat_entry* entry) {
    struct fat_entry* empty_loc = allocate(FAT_ENTRY_SIZE);
    if (entry != NULL) {
        empty_loc->fat_number = entry->fat_number;
        empty_loc->cluster_number = entry->cluster_number;
    }

    int found = 0;
    while (!found) {
        struct fat_entry* tmp = sfs_fat_read_fat_entry(sfs, empty_loc);
        if (tmp->fat_number != 0 || tmp->cluster_number != 0) {
            /* increment to the next entry */
            empty_loc->cluster_number++;

            /* if we've gone past the end of the FAT, increment the FAT number */
            if (!empty_loc->cluster_number /* overflowed */
                    || empty_loc->cluster_number > sfs->entries_per_fat) {
                empty_loc->cluster_number = 0;
                empty_loc->fat_number++;

                /* if fat number overflows, game over */
                if (!empty_loc->fat_number) {
                    abort();
                }
            }
        } else {
            found = 1;
        }
        free(tmp);
    }

    return (empty_loc);
}

/* @return the location of the first empty FAT entry */
HIDDEN struct fat_entry* sfs_fat_get_first_empty_fat_entry(
        const struct sfs_filesystem* sfs) {
    struct fat_entry* avail = allocate(FAT_ENTRY_SIZE);
    if (sfs->first_available_fat_entry != NULL) {
        avail->fat_number = sfs->first_available_fat_entry->fat_number;
        avail->cluster_number = sfs->first_available_fat_entry->cluster_number;
    }

    struct fat_entry* empty_entry = sfs_fat_read_fat_entry(sfs, avail);

    /* just making sure it's still empty */
    if (empty_entry->fat_number != 0 || empty_entry->cluster_number != 0) {
        /* something went wrong and this isn't actually empty */
        /* scan the whole FAT until we find an empty entry */
        free(empty_entry);
        empty_entry = sfs_fat_find_next_empty_fat_entry(sfs, &FREE_ENTRY);
        avail->fat_number = empty_entry->fat_number;
        avail->cluster_number = empty_entry->cluster_number;
    }
    free(empty_entry);

    return (avail);
}

HIDDEN int sfs_fat_allocate_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file) {
    SFS_FILE_LENGTH length = file->file_length;

    struct fat_entry* next_entry = sfs_fat_get_first_empty_fat_entry(sfs);
    struct fat_list* chain = allocate(sizeof(struct fat_list));
	chain->entry = next_entry;
    chain->next = NULL;

    uint32_t sizeof_cluster = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    if (length < sizeof_cluster) {
        length = 0;
    } else {
        length -= sizeof_cluster;
    }
    struct fat_list* tail = chain;
    while (length > 0) {
        next_entry = sfs_fat_find_next_empty_fat_entry(sfs, next_entry);
        tail->next = allocate(sizeof(struct fat_list));
        tail = tail->next;
        tail->entry = next_entry;
        tail->next = NULL;
        if (length < sizeof_cluster) {
            length = 0;
        } else {
            length -= sizeof_cluster;
        }
    }

    tail = chain;
    while (tail->next != NULL) {
        sfs_file_jump_to_cluster(sfs, tail->entry);
        sfs_io_write_new_cluster(sfs);
        sfs_fat_write_fat_entry(sfs, tail->entry, tail->next->entry);
        tail = tail->next;
    }

    sfs_fat_mark_as_end_of_chain(sfs, tail->entry);

    /* TODO for some reason this is messing with the allocation.
     * gotta figure out how to keep track of the first empty entry
     struct fat_entry* avail = sfs->first_available_fat_entry;
     struct fat_entry* empty_entry = sfs_fat_find_next_empty_fat_entry(sfs,
     chain->entry);
     avail->fat_number = empty_entry->fat_number;
     avail->cluster_number = empty_entry->cluster_number;
     */

    file->clusters = chain;
    file->current_cluster = chain;
    file->first_cluster = chain->entry;

    return (0);
}

/* @param the FAT entry corresponding to the final cluster in the file */
/* @return the FAT entry corresponding to the newly allocated cluster */
HIDDEN struct fat_entry* sfs_fat_allocate_cluster(
        const struct sfs_filesystem* sfs, const struct fat_entry* end_of_chain) {
    struct fat_entry* next_entry = sfs_fat_get_first_empty_fat_entry(sfs);
    sfs_fat_write_fat_entry(sfs, end_of_chain, next_entry);
    sfs_fat_mark_as_end_of_chain(sfs, next_entry);

    struct fat_entry* avail = sfs->first_available_fat_entry;
    struct fat_entry* empty_entry = allocate(FAT_ENTRY_SIZE);
    empty_entry = sfs_fat_find_next_empty_fat_entry(sfs, next_entry);
    avail->fat_number = empty_entry->fat_number;
    avail->cluster_number = empty_entry->cluster_number;

    sfs_file_jump_to_cluster(sfs, next_entry);
    sfs_io_write_new_cluster(sfs);

    free(empty_entry);

    return (next_entry);
}

HIDDEN struct fat_list* sfs_fat_get_cluster_chain(
        const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
    return (NULL);
}

HIDDEN int sfs_fat_write_new_fat(const struct sfs_filesystem* sfs) {
    const uint64_t sizeof_fat = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    uint8_t* fat = allocate(sizeof_fat);
    sfs_encrypt(fat, sfs->global_key, sizeof_fat);
    sfs_util_write_to_medium(sfs->fd, fat, sizeof_fat);
    free(fat);

    return (0);
}
