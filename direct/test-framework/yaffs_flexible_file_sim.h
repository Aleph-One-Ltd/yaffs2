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

/*
 * Flexible simple file based NAND Simulator for testing YAFFS
 */


#ifndef __YAFFS_FLEXZIBLE_FILE_SIM_H__
#define __YAFFS_FLEXZIBLE_FILE_SIM_H__

#include <stdint.h>

struct yaffs_dev;

struct yaffs_dev *yaffs_flexible_file_sim_create(
				const char *name,
				const char *sim_file_name,
				uint32_t n_blocks,
				uint32_t start_block, uint32_t end_block,
				uint32_t chunks_per_block,
				uint32_t bytes_per_chunk,
				uint32_t bytes_per_spare) ;
#endif
