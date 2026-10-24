/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __YAFFS_ENCRYPTION_H__
#define __YAFFS_ENCRYPTION_H__
#include "yaffs_guts.h"

/*
 *  Functions to initialise and deinitialise the encryption layer if one
 *  is installed. If not, installs a null encryptor.
 */
int yaffs_has_encryption_layer(struct yaffs_dev *dev);
int yaffs_encryption_init(struct yaffs_dev *dev);
int yaffs_encryption_deinit(struct yaffs_dev *dev);

/*
 * Wrapper functions to access encryptor functionality.
 */
int yaffs_encryption_get_reserved_bytes(struct yaffs_dev *dev);
const char * yaffs_encryption_get_name(struct yaffs_dev *dev);

int yaffs_encryption_encrypt(struct yaffs_dev *dev,
			     const u8 *data, int data_len);
int yaffs_encryption_decrypt(struct yaffs_dev *dev,
			     u8 *data, int data_len);
u8 *yaffs_encryption_get_read_buffer(struct yaffs_dev *dev,
				     u8 *buffer);
const u8 *yaffs_encryption_get_write_buffer(struct yaffs_dev *dev,
					    const u8 *buffer);
#endif
