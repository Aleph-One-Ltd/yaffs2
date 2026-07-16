/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "yaffs_encryption.h"

int yaffs_has_encryption_layer(struct yaffs_dev *dev)
{
	return (dev->param.inband_tags && dev->param.is_yaffs2);
}

int yaffs_encryption_init(struct yaffs_dev *dev)
{
	if (!yaffs_has_encryption_layer (dev)) {
		dev->enc.enc_reserved_bytes = 0;
		dev->enc.enc_name = "encryption_disabled";
		return 0;
	}

	if(dev->enc.enc_initialise_fn)
		return dev->enc.enc_initialise_fn(dev);

	/*
	 * Set up a NULL encryption layer that just passes through
	 * to the driver.
	 */
	dev->enc.enc_read_chunk_fn = dev->drv.drv_read_chunk_fn;
	dev->enc.enc_write_chunk_fn = dev->drv.drv_write_chunk_fn;
	dev->enc.enc_deinitialise_fn = NULL;
	dev->enc.enc_reserved_bytes = 0;
	dev->enc.enc_name = "null_encryption";
	return 0;
}

int yaffs_encryption_deinit(struct yaffs_dev *dev)
{
	if (yaffs_has_encryption_layer (dev) &&
	   dev->enc.enc_deinitialise_fn)
		return dev->enc.enc_deinitialise_fn(dev);

	return 0;
}
