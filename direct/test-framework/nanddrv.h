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

#ifndef __NAND_DRIVER_H__
#define __NAND_DRIVER_H__
#include "nand_chip.h"

struct nanddrv_transfer {
	unsigned char *buffer;
	int offset;
	int nbytes;
};

int nanddrv_read_tr(struct nand_chip *this, int page,
		struct nanddrv_transfer *tr, int n_tr);
int nanddrv_write_tr(struct nand_chip *this, int page,
		struct nanddrv_transfer *tr, int n_tr);
int nanddrv_erase(struct nand_chip *this, int block);

#endif

