/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __NAND_STORE_FILE_H__
#define __NAND_STORE_FILE_H__

#include "nand_store.h"

struct nand_store *
nandstore_file_init(const char *fname,
				int blocks,
				int pages_per_block,
				int data_bytes_per_page,
				int spare_bytes_per_page);

#endif
