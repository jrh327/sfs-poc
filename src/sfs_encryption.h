/*
 * sfs_encryption.h
 *
 *  Created on: Aug 14, 2017
 *      Author: jon
 */

#ifndef SFS_ENCRYPTION_H
#define SFS_ENCRYPTION_H

#include "sfs_includes.h"

#define ENCRYPTION_BLOCK_SIZE 16

/* not really sure how encryption library functions yet.
 * reallllly need to get around to testing that
 * encryption/decryption will be put into a new array
 */
HIDDEN uint8_t* sfs_encrypt(const uint8_t* buffer,
        const struct encryption_key* key, uint64_t length);

HIDDEN uint8_t* sfs_decrypt(const uint8_t* buffer,
        const struct encryption_key* key, uint64_t length);

#endif /* SFS_ENCRYPTION_H */
