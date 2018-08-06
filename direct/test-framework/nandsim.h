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

#ifndef __NAND_SIM_H__
#define __NAND_SIM_H__
#include "nand_store.h"
#include "nand_chip.h"
struct nand_chip *nandsim_init(struct nand_store *store, int bus_width_shift);
void nandsim_set_debug(int d);
#endif
