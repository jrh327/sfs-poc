/*
 * sfs_encryption.c
 *
 *  Created on: Aug 31, 2017
 *      Author: jon
 */

#include "sfs_includes.h"

HIDDEN uint8_t* sfs_encrypt(const uint8_t* buffer,
        const struct encryption_key* key, uint64_t length) {
    /* create copy of buffer
     * pass copy to encryption function
     * return encrypted copy of buffer
     *
     * TODO handle case of end of buffer not aligning with block
     * TODO add actual encryption back in. DO NOT rely on current "encryption"
     */
    uint8_t* encrypted = allocate(length);
    memcpy(encrypted, buffer, length);
    for (size_t i = 0; i < length; i++) {
        encrypted[i] = encrypted[i] - 57;
    }
    return (encrypted);
}

HIDDEN uint8_t* sfs_decrypt(const uint8_t* buffer,
        const struct encryption_key* key, uint64_t length) {
    /* create copy of buffer
     * iterate over length, decrypting into copy using key
     * return decrypted copy of buffer
     */
    uint8_t* decrypted = allocate(length);
    memcpy(decrypted, buffer, length);
    for (size_t i = 0; i < length; i++) {
        decrypted[i] = decrypted[i] + 57;
    }
    return (decrypted);
}
