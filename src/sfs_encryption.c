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
     */
    return (NULL);
}

HIDDEN uint8_t* sfs_decrypt(const uint8_t* buffer,
        const struct encryption_key* key, uint64_t length) {
    /* create copy of buffer
     * iterate over length, decrypting into copy using key
     * return decrypted copy of buffer
     */
    return (NULL);
}
