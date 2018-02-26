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

#include "nandsim_file.h"
#include "nandstore_file.h"
#include "nandsim.h"
#include <unistd.h>


struct nand_chip *nandsim_file_init(const char *fname,
				int blocks,
				int pages_per_block,
				int data_bytes_per_page,
				int spare_bytes_per_page,
				int bus_width_shift)
{
	struct nand_store *store = NULL;
	struct nand_chip *chip = NULL;

	store = nandstore_file_init(fname, blocks, pages_per_block,
					data_bytes_per_page,
					spare_bytes_per_page);
	if(store)
		chip = nandsim_init(store, bus_width_shift);

	if(chip)
		return chip;

	if(store){
		/* tear down */
	}
	return NULL;
}
