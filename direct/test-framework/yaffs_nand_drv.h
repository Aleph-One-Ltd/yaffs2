/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __YAFFS_NAND_DRV_H__
#define __YAFFS_NAND_DRV_H__

struct nand_store;

struct yaffs_dev *
	yaffs_nand_install_drv(const char *name,
				struct nand_store *ns);

struct yaffs_dev *yaffs_nandsim_install_drv(const char *name,
					const char *file_name,
					int n_blocks);

#endif
