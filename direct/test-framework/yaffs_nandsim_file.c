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


#include "yaffs_nandsim_file.h"
#include "yaffs_nand_drv.h"
#include "nandsim_file.h"
#include "nand_chip.h"
#include "yaffs_guts.h"
#include <stddef.h>


struct yaffs_dev *yaffs_nandsim_install_drv(const char *dev_name,
					const char *backing_file_name,
					int n_blocks)
{
	struct yaffs_dev *dev;
	char *name_copy = NULL;
	struct yaffs_param *param;
	struct nand_chip *chip = NULL;



	dev = malloc(sizeof(struct yaffs_dev));
	name_copy = strdup(dev_name);

	if(!dev || !name_copy)
		goto fail;

	memset(dev, 0, sizeof(*dev));
	chip = nandsim_file_init(backing_file_name, n_blocks, 64, 2048, 64, 0);
	if(!chip)
		goto fail;

	param = &dev->param;

	param->name = name_copy;

	param->total_bytes_per_chunk = chip->data_bytes_per_page;
	param->chunks_per_block = chip->pages_per_block;
	param->n_reserved_blocks = 5;
	param->start_block = 0; // First block
	param->end_block = n_blocks - 1; // Last block
	param->is_yaffs2 = 1;
	param->use_nand_ecc = 1;
	param->n_caches = 10;
	param->stored_endian = 2;

	if(yaffs_nand_install_drv(dev, chip) != YAFFS_OK)
		goto fail;

	/* The yaffs device has been configured, install it into yaffs */
	yaffs_add_device(dev);

	return dev;

fail:
	free(dev);
	free(name_copy);
	return NULL;
}
