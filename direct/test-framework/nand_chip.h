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

#ifndef __NAND_CHIP_H__
#define __NAND_CHIP_H__

#include <stdint.h>

struct nand_chip {
	void *private_data;

	void (*set_ale)(struct nand_chip * this, int high);
	void (*set_cle)(struct nand_chip * this, int high);

	unsigned (*read_cycle)(struct nand_chip * this);
	void (*write_cycle)(struct nand_chip * this, unsigned b);

	int (*check_busy)(struct nand_chip * this);
	void (*idle_fn) (struct nand_chip *this);
	int (*power_check) (struct nand_chip *this);

	int blocks;
	int pages_per_block;
	int data_bytes_per_page;
	int spare_bytes_per_page;
	int bus_width_shift;
};
#endif
