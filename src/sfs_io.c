/*
 * sfs_io.c
 *
 *  Created on: Aug 24, 2017
 *      Author: Jon Hopkins
 */

#include "sfs_includes.h"

HIDDEN struct fat_entry* sfs_io_read_fat_entry(const struct sfs_filesystem* sfs) {
    /* distance from the beginning of the current encryption block */
    /*
     * since we know ENCRYPTION_BLOCK_SIZE will always be 2^n, then
     * instead of using the expensive modulo operator we can do a
     * bitwise AND of the last n bits to get the modulus.
     */
    SFS_FILE_OFFSET offset = (sfs_util_tell_file(sfs->fd)
            & (ENCRYPTION_BLOCK_SIZE - 1));
    if (offset) {
        /* jump backwards to align with current encryption block */
        sfs_util_seek_in_medium(sfs->fd, -offset, SEEK_CUR);
    }

    /* get the whole block */
    uint8_t* block = allocate(ENCRYPTION_BLOCK_SIZE);
    sfs_util_read_from_medium(sfs->fd, block, ENCRYPTION_BLOCK_SIZE);

    sfs_decrypt(block, sfs->global_key, ENCRYPTION_BLOCK_SIZE);

    /* get the correct bytes from the block */
    struct fat_entry* entry = allocate(FAT_ENTRY_SIZE);
    uint8_t* tmp = block + offset;
    entry->fat_number = sfs_util_get_uint16(tmp, 0);
    entry->cluster_number = sfs_util_get_uint16(tmp, 2);

    free(block);

    return (entry);
}

HIDDEN int sfs_io_write_fat_entry(const struct sfs_filesystem* sfs,
        const struct fat_entry* entry) {
    SFS_FILE_OFFSET offset = (sfs_util_tell_file(sfs->fd)
            & (ENCRYPTION_BLOCK_SIZE - 1));
    if (offset) {
        sfs_util_seek_in_medium(sfs->fd, -offset, SEEK_CUR);
    }

    uint8_t* block = allocate(ENCRYPTION_BLOCK_SIZE);
    sfs_util_read_from_medium(sfs->fd, block, ENCRYPTION_BLOCK_SIZE);

    sfs_decrypt(block, sfs->global_key, ENCRYPTION_BLOCK_SIZE);

    /* put the new bytes into the block */
    uint8_t* tmp = block + offset;
    sfs_util_put_uint16(tmp, entry->fat_number, 0);
    sfs_util_put_uint16(tmp, entry->cluster_number, 2);

    /* encrypt the block with the new bytes in it */
    sfs_encrypt(tmp, sfs->global_key, ENCRYPTION_BLOCK_SIZE);

    /* jump back to the start of the block and rewrite it */
    sfs_util_seek_in_medium(sfs->fd, -ENCRYPTION_BLOCK_SIZE, SEEK_CUR);
    sfs_util_write_to_medium(sfs->fd, block, ENCRYPTION_BLOCK_SIZE);

    free(block);

    return (0);
}

HIDDEN int sfs_io_write_new_fat(const struct sfs_filesystem* sfs) {
    const uint64_t sizeof_fat = sfs->entries_per_fat * FAT_ENTRY_SIZE;
    uint8_t* fat = allocate(sizeof_fat);
    sfs_encrypt(fat, sfs->global_key, sizeof_fat);
    sfs_util_write_to_medium(sfs->fd, fat, sizeof_fat);
    free(fat);

    return (0);
}

HIDDEN int sfs_io_read_cluster(const struct sfs_filesystem* sfs,
        const struct encryption_key* key, uint8_t* buffer,
        const SFS_FILE_LENGTH length) {
    uint8_t* ptr = buffer;
    SFS_FILE_LENGTH bytes_left = length;

    /* manually keep track of the position instead of keep calling ftell */
    SFS_FILE_OFFSET curpos = sfs_util_tell_file(sfs->fd);
    SFS_FILE_OFFSET offset = (curpos & (ENCRYPTION_BLOCK_SIZE - 1));
    if (offset) {
        sfs_util_seek_in_medium(sfs->fd, -offset, SEEK_CUR);
        curpos -= offset;

        uint8_t* encrypted_block = allocate(ENCRYPTION_BLOCK_SIZE);
        sfs_util_read_from_medium(sfs->fd, encrypted_block,
                ENCRYPTION_BLOCK_SIZE);
        curpos += ENCRYPTION_BLOCK_SIZE;

        uint8_t* decrypted_block = sfs_decrypt(encrypted_block, key,
                ENCRYPTION_BLOCK_SIZE);
        free(encrypted_block);

        /*
         * get the correct bytes from the block, even if fewer bytes were
         * requested than are in the block
         */
        size_t bytes_in_block = (ENCRYPTION_BLOCK_SIZE - offset);
        if (bytes_in_block > bytes_left) {
            bytes_in_block = bytes_left;
        }

        uint8_t* tmp = decrypted_block + offset;
        for (size_t i = 0; i < bytes_in_block; i++) {
            *ptr++ = *tmp++;
        }
        free(decrypted_block);

        if (bytes_in_block >= bytes_left) {
            return (ptr - buffer);
        }

        /* adjust for the bytes we just read */
        ptr += bytes_in_block;
        bytes_left -= (ENCRYPTION_BLOCK_SIZE - offset);
    }

    /* read as many full encryption blocks as possible with what's left */
    SFS_FILE_OFFSET extra_bytes = (bytes_left & (ENCRYPTION_BLOCK_SIZE - 1));
    bytes_left -= extra_bytes;
    if (bytes_left) {
        uint8_t* encrypted_block = allocate(bytes_left);
        sfs_util_read_from_medium(sfs->fd, encrypted_block, bytes_left);
        curpos += bytes_left;
        uint8_t* decrypted_block = sfs_decrypt(encrypted_block, key,
                bytes_left);
        free(encrypted_block);

        memcpy(ptr, decrypted_block, bytes_left);
        free(decrypted_block);
        ptr += bytes_left;
    }

    if (extra_bytes) {
        bytes_left = extra_bytes;
        uint8_t* encrypted_block = allocate(ENCRYPTION_BLOCK_SIZE);
        sfs_util_read_from_medium(sfs->fd, encrypted_block,
                ENCRYPTION_BLOCK_SIZE);
        uint8_t* decrypted_block = sfs_decrypt(encrypted_block, key,
                ENCRYPTION_BLOCK_SIZE);

        uint8_t* tmp = decrypted_block;
        for (size_t i = 0; i < bytes_left; i++) {
            *ptr++ = *tmp++;
        }
        free(decrypted_block);
    }

    return (ptr - buffer);
}

HIDDEN int sfs_io_write_cluster(const struct sfs_filesystem* sfs,
        const struct encryption_key* key, const uint8_t* data,
        const SFS_FILE_LENGTH length) {
    uint8_t* ptr = data;
    SFS_FILE_LENGTH bytes_left = length;

    /* manually keep track of the position instead of keep calling ftell */
    SFS_FILE_OFFSET curpos = sfs_util_tell_file(sfs->fd);
    SFS_FILE_OFFSET offset = (curpos & (ENCRYPTION_BLOCK_SIZE - 1));
    if (offset) {
        sfs_util_seek_in_medium(sfs->fd, -offset, SEEK_CUR);
        curpos -= offset;

        uint8_t* encrypted_block = allocate(ENCRYPTION_BLOCK_SIZE);
        sfs_util_read_from_medium(sfs->fd, encrypted_block,
                ENCRYPTION_BLOCK_SIZE);
        curpos += ENCRYPTION_BLOCK_SIZE;

        uint8_t* decrypted_block = sfs_decrypt(encrypted_block, key,
                ENCRYPTION_BLOCK_SIZE);
        free(encrypted_block);

        /*
         * get the correct bytes from the block, even if fewer bytes were
         * requested than are in the block
         */
        size_t bytes_in_block = (ENCRYPTION_BLOCK_SIZE - offset);
        if (bytes_in_block > bytes_left) {
            bytes_in_block = bytes_left;
        }

        uint8_t* tmp = decrypted_block + offset;
        for (size_t i = 0; i < bytes_in_block; i++) {
            *tmp++ = *ptr++;
        }

        sfs_util_seek_in_medium(sfs->fd, -ENCRYPTION_BLOCK_SIZE, SEEK_CUR);

        encrypted_block = sfs_encrypt(decrypted_block, key,
                ENCRYPTION_BLOCK_SIZE);
        free(decrypted_block);

        sfs_util_write_to_medium(sfs->fd, encrypted_block,
                ENCRYPTION_BLOCK_SIZE);
        free(encrypted_block);

        if (bytes_in_block >= bytes_left) {
            return (ptr - data);
        }

        /* adjust for the bytes we just wrote */
        bytes_left -= (ENCRYPTION_BLOCK_SIZE - offset);
    }

    /* write as many full encryption blocks as possible with what's left */
    SFS_FILE_OFFSET extra_bytes = (bytes_left & (ENCRYPTION_BLOCK_SIZE - 1));
    bytes_left -= extra_bytes;
    if (bytes_left) {
        uint8_t* encrypted_block = sfs_encrypt(ptr, key, bytes_left);

        sfs_util_write_to_medium(sfs->fd, encrypted_block, bytes_left);
        free(encrypted_block);
        curpos += bytes_left;

        ptr += bytes_left;
    }

    /* write any extra bytes into beginning of next encryption block */
    if (extra_bytes) {
        bytes_left = extra_bytes;
        uint8_t* encrypted_block = allocate(ENCRYPTION_BLOCK_SIZE);
        SFS_FILE_LENGTH bytes_read = sfs_util_read_from_medium(sfs->fd,
                encrypted_block, ENCRYPTION_BLOCK_SIZE);
        uint8_t* decrypted_block = sfs_decrypt(encrypted_block, key,
                bytes_read);
        free(encrypted_block);
        sfs_util_seek_in_medium(sfs->fd, -bytes_read, SEEK_CUR);

        uint8_t* tmp = decrypted_block;
        for (size_t i = 0; i < bytes_left; i++) {
            /*ptr[i] = block[i];*/
            *tmp++ = *ptr++;
        }

        encrypted_block = sfs_encrypt(decrypted_block, key,
                ENCRYPTION_BLOCK_SIZE);
        free(decrypted_block);

        sfs_util_write_to_medium(sfs->fd, encrypted_block,
                ENCRYPTION_BLOCK_SIZE);
        free(encrypted_block);
    }

    return (ptr - data);
}

HIDDEN int sfs_io_write_new_cluster(const struct sfs_filesystem* sfs) {
    const uint64_t sizeof_sector = sfs->bytes_per_sector;
    uint8_t* empty_sector = allocate(sizeof_sector);
    uint8_t* encrypted_sector = sfs_encrypt(empty_sector, sfs->global_key,
            sizeof_sector);
    free(empty_sector);

    size_t num_sectors = sfs->sectors_per_cluster;
    for (size_t i = 0; i < num_sectors; i++) {
        sfs_util_write_to_medium(sfs->fd, encrypted_sector, sizeof_sector);
    }
    free(encrypted_sector);

    return (0);
}
