/*
 * sfs_file.c
 *
 *  Created on: Aug 14, 2017
 *      Author: Jon Hopkins
 */

#include <limits.h>
#include "sfs_includes.h"

HIDDEN int sfs_file_jump_to_cluster(const struct sfs_filesystem* sfs,
        const struct fat_entry* cluster) {
    const uint64_t begin_data = BOOT_SECTOR_SIZE;
    const uint64_t sizeof_fat = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    const uint64_t sizeof_cluster = sfs->bytes_per_sector
            * sfs->sectors_per_cluster;
    const uint16_t clusters_per_data_block = sfs->entries_per_fat;
    const uint32_t sizeof_data_block = sizeof_fat
            + sizeof_cluster * clusters_per_data_block;

    int fd = sfs->fd;

    off_t location = begin_data;
    location += (cluster->fat_number * sizeof_data_block) + sizeof_fat;
    location += (cluster->cluster_number * sizeof_cluster);

    /* check if going past the current end of the filesystem */
    /*sfs_util_seek_in_medium(fd, 0, SEEK_END);
     uint64_t size = sfs_util_tell_file(fd);*/
    sfs_util_seek_in_medium(fd, location, SEEK_SET);

    /* generate the cluster if we went past the end of the existing filesystem */
    /*if (size <= location) {
        sfs_io_write_new_cluster(sfs);
     }*/

    off_t ret = sfs_util_seek_in_medium(fd, location, SEEK_SET);
    if (ret == -1) {
        return (-1);
    }

    return (0);
}

HIDDEN int sfs_file_seek_in_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file, SFS_FILE_OFFSET offset, int whence) {
    if (whence == SEEK_END) {
        offset = file->file_length + offset;
        file->current_cluster->entry = file->first_cluster;
    } else if (whence == SEEK_CUR) {
        /* nothing to do here */
    } else if (whence == SEEK_SET) {
        file->current_cluster->entry = file->first_cluster;
    } else {
        return (-1);
    }

    if (offset < 0) {
        return (-1);
    }

    file->current_offset = 0;
    uint16_t cluster_size = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    /* if seeking past the end of the file, will need to allocate clusters on next read/write */
    while (file->current_offset < whence && file->current_cluster->next) {
        file->current_cluster = file->current_cluster->next;
        file->current_offset += cluster_size;
    }
    file->current_offset = offset;

    return (0);
}

static SFS_FILE_LENGTH sfs_file_bytes_left_in_cluster(
        const struct sfs_filesystem* sfs,
        const struct fat_entry* current_cluster) {
    SFS_FILE_OFFSET pos = sfs_util_tell_file(sfs->fd);
    uint64_t sizeof_fat = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    uint64_t sizeof_cluster = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint16_t clusters_per_data_block = sfs->entries_per_fat;
    uint32_t sizeof_data_block = sizeof_fat
            + sizeof_cluster * clusters_per_data_block;

    uint64_t start_data_block = current_cluster->fat_number * sizeof_data_block;
    uint64_t start_cluster = start_data_block
            + current_cluster->cluster_number * sizeof_cluster;

    SFS_FILE_LENGTH cluster_pos = pos - start_cluster;

    return (cluster_pos);
}

HIDDEN int sfs_file_read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, uint8_t* buffer,
        SFS_FILE_LENGTH length) {
    sfs_file_jump_to_cluster(sfs, file->current_cluster->entry);

    uint32_t sizeof_cluster = sfs->bytes_per_sector * sfs->sectors_per_cluster;
    uint32_t offset_in_cluster = file->current_offset % sizeof_cluster;
    if (offset_in_cluster) {
        sfs_util_seek_in_medium(sfs->fd, offset_in_cluster, SEEK_CUR);
    }

    /* file already seeked to position */
    SFS_FILE_LENGTH bytes_left = length;
    struct fat_list* current_cluster = file->current_cluster;
    while (bytes_left) {
        SFS_FILE_LENGTH left_in_cluster = sfs_file_bytes_left_in_cluster(sfs,
                current_cluster->entry);
        SFS_FILE_LENGTH bytes_to_read = bytes_left;
        if (bytes_to_read > left_in_cluster) {
            bytes_to_read = left_in_cluster;
        }

        int bytes_read = sfs_io_read_cluster(sfs, file->key, buffer,
                bytes_to_read);
        if (bytes_read == -1) {
            return (-1);
        }
        bytes_left -= bytes_read;
        current_cluster = current_cluster->next;
    }

    return (0);
}

HIDDEN int sfs_file_write_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, const uint8_t* data,
        const SFS_FILE_LENGTH length) {
    SFS_FILE_LENGTH bytes_left = length;
    struct fat_list* current_cluster = file->current_cluster;

    while (bytes_left) {
        SFS_FILE_LENGTH left_in_cluster = sfs_file_bytes_left_in_cluster(sfs,
                current_cluster->entry);
        SFS_FILE_LENGTH bytes_to_write = bytes_left;
        if (bytes_to_write > left_in_cluster) {
            bytes_to_write = left_in_cluster;
        }

        int bytes_written = sfs_io_write_cluster(sfs, file->key, data,
                bytes_to_write);
        if (bytes_written == -1) {
            return (-1);
        }

        bytes_left -= bytes_written;
        current_cluster = current_cluster->next;
    }

    return (0);
}
