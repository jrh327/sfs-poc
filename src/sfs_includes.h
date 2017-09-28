/*
 * sfs_includes.h
 *
 *  Created on: Aug 14, 2017
 *      Author: jon
 */

#ifndef SFS_INCLUDES_H
#define SFS_INCLUDES_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define EXPORT __attribute__((visibility("default")))
#define HIDDEN __attribute__((visibility("hidden")))
#define allocate(x) calloc(1, x)

#include "sfs_structs.h"
#include "sfs.h"
#include "sfs_directory.h"
#include "sfs_encryption.h"
#include "sfs_fat.h"
#include "sfs_file.h"
#include "sfs_io.h"
#include "sfs_util.h"

#endif /* SFS_INCLUDES_H */
