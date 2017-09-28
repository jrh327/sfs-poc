/*
 * sfs.c
 *
 *  Created on: Aug 14, 2017
 *      Author: Hopkins
 */

#include "sfs_includes.h"

EXPORT struct sfs_filesystem* sfs_initialize_new_filesystem(int fd,
        uint16_t fat_size, uint16_t bytes_per_sector,
        uint8_t sectors_per_cluster) {
    return (sfs_initialize_filesystem_partition(fd, 0, fat_size,
            bytes_per_sector,
            sectors_per_cluster));
}

EXPORT struct sfs_filesystem* sfs_initialize_filesystem_partition(int fd,
        uint64_t partition_offset, uint16_t fat_size, uint16_t bytes_per_sector,
        uint8_t sectors_per_cluster) {
    if (fat_size != FAT_SIZE_SMALL && fat_size != FAT_SIZE_MEDIUM
            && fat_size != FAT_SIZE_LARGE) {
        fat_size = FAT_SIZE_MEDIUM;
    }

    if (bytes_per_sector < MIN_BYTES_PER_SECTOR) {
        bytes_per_sector = MIN_BYTES_PER_SECTOR;
    } else {
        uint16_t bitpos = MAX_BYTES_PER_SECTOR;
        while (!(bytes_per_sector & bitpos)) {
            bitpos = bitpos >> 1;
        }
        bytes_per_sector = bitpos;
    }

    if (sectors_per_cluster * bytes_per_sector > MAX_BYTES_PER_CLUSTER) {
        sectors_per_cluster = MAX_BYTES_PER_CLUSTER / bytes_per_sector;
    } else if (sectors_per_cluster == 0) {
        sectors_per_cluster = 1;
    } else {
        uint8_t bitpos = MAX_SECTORS_PER_CLUSTER;
        while (!(sectors_per_cluster & bitpos)) {
            bitpos = bitpos >> 1;
        }
        sectors_per_cluster = bitpos;
    }

    struct sfs_filesystem* sfs = allocate(sizeof(struct sfs_filesystem));
    sfs->fd = fd;
    sfs->partition_offset = partition_offset;
    sfs->entries_per_fat = fat_size;
    sfs->bytes_per_sector = bytes_per_sector;
    sfs->sectors_per_cluster = sectors_per_cluster;

    uint8_t arr_boot_sector[BOOT_SECTOR_SIZE] = {
            "SFS v1.0"
    };

    sfs_util_put_uint64(arr_boot_sector, sfs->partition_offset, 8);
    sfs_util_put_uint16(arr_boot_sector, sfs->entries_per_fat, 16);
    sfs_util_put_uint16(arr_boot_sector, sfs->bytes_per_sector, 18);
    arr_boot_sector[20] = sfs->sectors_per_cluster;

    sfs_util_write_to_medium(fd, arr_boot_sector, BOOT_SECTOR_SIZE);

    /* initialize the file allocation table */
    uint8_t* arr_fat_sector = allocate(sfs->bytes_per_sector);
    size_t num_fat_sectors = (sfs->entries_per_fat * FAT_ENTRY_SIZE)
            / sfs->bytes_per_sector;
    for (size_t i = 0; i < num_fat_sectors; i++) {
        sfs_util_write_to_medium(fd, arr_fat_sector, sfs->bytes_per_sector);
    }
    free(arr_fat_sector);

    sfs->first_available_fat_entry = sfs_fat_get_first_empty_fat_entry(sfs);

    /* initialize a cluster for the root directory */
    /* generated automatically by the jump */
    struct fat_entry* first_cluster = sfs_fat_allocate_cluster(sfs,
            sfs_fat_get_first_empty_fat_entry(sfs));

    sfs_file_jump_to_cluster(sfs, first_cluster);
    free(first_cluster);

    return (sfs);
}

EXPORT struct sfs_filesystem* sfs_load_filesystem(int fd) {
    /* read the first three bytes to check if this is an SFS filesystem */
    if (sfs_util_read_uint8(fd) != 'S' || sfs_util_read_uint8(fd) != 'F'
            || sfs_util_read_uint8(fd) != 'S') {
        printf("Given file does not represent an SFS filesystem.\n");
        return (NULL);
    }

    /* skip to the actual data */
    sfs_util_seek_in_medium(fd, 8, SEEK_SET);

    struct sfs_filesystem* sfs = allocate(sizeof(struct sfs_filesystem));
    sfs->fd = fd;
    sfs->partition_offset = sfs_util_read_uint64(fd);
    sfs->entries_per_fat = sfs_util_read_uint16(fd);
    sfs->bytes_per_sector = sfs_util_read_uint16(fd);
    sfs->sectors_per_cluster = sfs_util_read_uint8(fd);
    sfs->first_available_fat_entry = sfs_fat_get_first_empty_fat_entry(sfs);

    return (sfs);
}

EXPORT int sfs_close_filesystem(struct sfs_filesystem* sfs) {
    int error = sfs_util_close_medium(sfs->fd);

    if (!error) {
        free_sfs(sfs);
    }

    return (error);
}

EXPORT struct directory_entry* sfs_get_root_directory(
        const struct sfs_filesystem* sfs) {
    return (sfs_dir_get_root_directory_entry(sfs));
}

EXPORT struct directory_list* sfs_list_directory(
        const struct sfs_filesystem* sfs,
        const struct directory_entry* dir) {
/*
list the contents of a directory
given the filesystem to read from
      the directory to scan for entries
do
- allocate head of linked_list<dir_entry>
- read directory's FAT chain
- loop for entry in FAT chain
  - jump to entry's location
  - loop for sizeof dir_entry
    - read entry into new dir_entry
      - load properties into dir_entry attributes
      - allocate filename[8 + (sizeof(dir_entry) - 1) * num_extra_entries (+ 1 for null terminator?)]
        // miiiiiight create filename struct with byte* and int length (number of bytes not including null terminator)
        // actually definitely going to do that: struct filename { int length; uint8* name };
        // will allow for validation on number of extra entries and total filename length
        //   enforcing 0s in unused bytes at end of last dir_entry
        //   allow client to decide what to do with the string
      - copy filename bytes into dir_entry's filename
    - if extra entries
      - loop for extra entries
        - validate first byte as demarcating next entry in filename chain
        - copy rest of bytes into dir_entry's filename
return 0 if successful, or an error code
*/
    return (sfs_dir_get_directory_contents(sfs, dir));
}

EXPORT struct file_stat* sfs_describe_file(const struct sfs_filesystem* sfs,
        const int fd) {
    return (sfs_dir_describe_file(sfs, fd));
}

EXPORT struct file_stat* sfs_describe_file_in_directory(
        const struct sfs_filesystem* sfs, const int dir, const char* filename) {
    return (sfs_dir_describe_file_in_directory(sfs, dir, filename));
}

EXPORT struct directory_entry* sfs_create_directory(
        const struct sfs_filesystem* sfs,
        const struct directory_entry* parent, const char* filename) {
    return (NULL);
}

EXPORT struct directory_entry* sfs_create_file(const struct sfs_filesystem* sfs,
        struct directory_entry* parent, const char* filename,
        const uint8_t* data, const SFS_FILE_LENGTH file_length) {
/*
create a new file
given the filesystem to write to
      the directory to create the file in
      the name of the new file
      the contents of the file
      the number of bytes in the file
do
- jump to start of directory
- jump to first free directory entry
- calculate number of extra entries needed
- loop scan directory entries until a space large enough to fit the name is found
  - store current position
  - set flag true
  - loop for 1 to number extra entries
    - if non-empty entry, set flag false and break
    - if entry contains nothing but 0s, end of used entries, can't have non-empty entries after, so break
    - if end of directory's current allocation, can't have any non-empty entries after, so break
  - if flag is true
    - create directory entry
    - write position in parent into directory entry
    - write directory entry
    - allocate file, store returned FAT chain, write first FAT entry to directory entry
    - write file
  - else
    - continue - reading will place us at the next directory entry anyway
- loop for entry in FAT chain
  - jump to entry's location
  - write contents to cluster
return the directory entry of the newly created file
 */
    struct directory_entry* entry = sfs_dir_create_directory_entry(parent,
            filename, file_length);

    sfs_fat_allocate_file(sfs, entry);

    entry->key = parent->key;

    sfs_dir_write_directory_entry(sfs, parent, entry);

    /* newly created file, so start writing at the beginning */
    sfs_file_jump_to_cluster(sfs, entry->first_cluster);

    sfs_file_write_file(sfs, entry, data, file_length);

    return (entry);
}

EXPORT int sfs_seek_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file, SFS_FILE_OFFSET offset, int whence) {
    int res = sfs_file_seek_in_file(sfs, file, offset, whence);
    return (res);
}

EXPORT int sfs_read_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file, uint8_t* buffer, uint64_t length) {
    int res = sfs_file_read_file(sfs, file, buffer, length);
    return (res);
}

EXPORT int sfs_delete_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file) {
/*
"delete" a file (probably rename as recycle or send_to_trash)
given the filesystem to delete from
      the file to delete
do
- jump to directory entry's location in parent directory (will be adding offset within parent to dir_entry struct)
- flag entry(s) as deleted
return 0 if successful, or an error code
 */
    file->reserved |= 0x80;
    for (int i = 0; i < file->filename_entries; i++) {
        /* get next filename entry */
        /* filename_entry->reserved |= 0x80; */
    }
    return (0);
}

EXPORT int sfs_undelete_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file) {
/*
restore a deleted file
given the filesystem to undelete from
      the file to undelete
do
- jump to directory entry's location in parent directory
- unflag entry(s) as deleted
return 0 if successful, or an error code
 */
    file->reserved &= (~SOFT_DELETED_DIR_ENTRY);
    int ret = sfs_dir_update_directory_entry(sfs, file);

    return (ret);
}

EXPORT int sfs_hard_delete_file(const struct sfs_filesystem* sfs,
        const struct directory_entry* file) {
/*
delete a file and remove all traces of it from the filesystem
given the filesystem to delete from
      the file to delete
do
- read file's FAT chain
- loop for entry in FAT chain
  - jump to entry's location
  - write 0s to all bytes in cluster
> truncate_fat_chain(dir_entry->fat_entry)
> mark_as_free(dir_entry->fat_entry)
- jump to directory entry's location in parent directory
- write 0s to all directory entries
  - if not last entry in parent, flag zeroed entries as hard-deleted
return 0 if successful, or an error code
 */
    /* follow cluster chain, zero out all bytes */
    /* mark as deleted and zero out all bytes of directory entry */
    return (0);
}

EXPORT int sfs_rename_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file,
        const char* filename) {
/*
rename a file
given the filesystem to write to
      the file to rename
      the new filename
do
- jump to directory entry's location in parent directory
- calculate number of extra entries needed by new filename
- if same or less than current extra entries
  - write new filename
  - zero out extra bytes in entry containing end of filename
  - zero out any unneeded entries at end
    - if not last entry in parent, flag zeroed entries as hard-deleted
- else
  - jump to first entry after current entry
  - set flag true
  - loop for 1 to (needed extra entries - current extra entries)
    - if non-empty entry, set flag false and break
    - if entry contains nothing but 0s, end of used entries, can't have non-empty entries after, so break
    - if end of directory's current allocation, can't have any non-empty entries after, so break
  - if flag is true
    - write new filename and return
  - loop scan directory entries until a space large enough to fit the name is found
    - store current position
    - set flag true
    - loop for 0 to number extra entries // start from 0 to accomodate actual dir_entry
      - if non-empty entry, set flag false and break
      - if entry contains nothing but 0s, end of used entries, can't have non-empty entries after, so break
      - if end of directory's current allocation, can't have any non-empty entries after, so break
    - if flag is true
      - write directory entry
return 0 if successful, or an error code
 */
    int old_filename_entries = file->filename_entries;
    sfs_dir_change_file_name(sfs, file, filename);

    if (file->filename_entries > old_filename_entries) {
        /* write new filename to end of entry list (or first open segment big enough to fit it) */
        /* mark all old entries as deleted (and maybe zero out all but first byte */
    } else {
        /* write new filename entries */
        /* mark any extra entries as deleted (and maybe zero out all but first byte) */
    }

    return (0);
}

EXPORT int sfs_move_file(const struct sfs_filesystem* sfs,
        struct directory_entry* file,
        const struct directory_entry* new_parent) {
/*
move a file to a different directory
given the filesystem to write to
      the file to move
      the new directory to move the file to
do
- jump to start of new directory
- jump to first free directory entry
- calculate number of extra entries needed
- loop scan directory entries until a space large enough to fit the name is found
  - store current position
  - set flag true
  - loop for 1 to number extra entries
    - if non-empty entry, set flag false and break
    - if entry contains nothing but 0s, end of used entries, can't have non-empty entries after, so break
    - if end of directory's current allocation, can't have any non-empty entries after, so break
  - if flag is true
    - write directory entry
    - write 0s to directory entry in old parent
      - if not last entry in old parent, flag zeroed entries as hard-deleted
return 0 if successful, or an error code
 */
    /* write directory entry to new parent */
    /* mark old entry as deleted */
    /* zero out all but first byte of entry(s) in old parent */
    return (0);
}
