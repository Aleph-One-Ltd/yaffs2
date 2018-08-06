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

#ifndef __YAFFS_NAND_DRV_H__
#define __YAFFS_NAND_DRV_H__

#include "yaffs_guts.h"
#include "nand_chip.h"

int yaffs_nand_install_drv(struct yaffs_dev *dev, struct nand_chip *chip);

#endif
