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
 * NAND Simulator for testing YAFFS
 */


#include "yaffs_flexible_file_sim.h"

#include "yaffs_guts.h"
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>


struct sim_data {
	char *file_name;
	int handle;
	uint32_t n_blocks;
	uint32_t chunks_per_block;
	uint32_t chunk_size;
	uint32_t spare_size;
	uint32_t total_chunk_size;
	uint32_t total_bytes_per_block;
	char *buffer;
};

static struct sim_data *dev_to_sim(struct yaffs_dev *dev)
{
	return (struct sim_data *)(dev->driver_context);
}


static int yflex_erase_internal(struct sim_data *sim, uint32_t block_id)
{
	int pos;

	pos = sim->total_bytes_per_block * block_id;
	memset(sim->buffer, 0xff, sim->total_bytes_per_block);

	lseek(sim->handle, pos, SEEK_SET);
	write(sim->handle, sim->buffer, sim->total_bytes_per_block);

	return YAFFS_OK;
}

#if 0
static int yflex_read_internal(struct sim_data *sim,
				 uint32_t page_id,
				 uint8_t *buffer)
{
	int page_pos = page_id * sim->total_chunk_size;

	lseek(sim->handle, page_pos, SEEK_SET);
	read(sim->handle, buffer, sim->total_chunk_size);

	return YAFFS_OK;
}

#endif


static int yflex_initialise(struct yaffs_dev *dev)
{
	(void) dev;

	return YAFFS_OK;
}


static int yflex_deinitialise(struct yaffs_dev *dev)
{
	(void) dev;
	return YAFFS_OK;
}

static int yflex_rd_chunk (struct yaffs_dev *dev, int page_id,
					  u8 *data, int data_length,
					  u8 *spare, int spare_length,
					  enum yaffs_ecc_result *ecc_result)
{
	struct sim_data *sim = dev_to_sim(dev);
	uint32_t page_start;

	page_start = page_id * sim->total_chunk_size;

	if(data) {
		lseek(sim->handle, page_start, SEEK_SET);
		read(sim->handle, data,data_length);
	}

	if(spare) {
		lseek(sim->handle, page_start + sim->chunk_size, SEEK_SET);
		read(sim->handle, spare,spare_length);
	}

	if (ecc_result)
		*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	return YAFFS_OK;
}

static int yflex_wr_chunk (struct yaffs_dev *dev, int page_id,
			     const u8 *data, int data_length,
			     const u8 *spare, int spare_length)
{
	struct sim_data *sim = dev_to_sim(dev);
	uint32_t page_start;

	page_start = page_id * sim->total_chunk_size;

	if(data) {
		lseek(sim->handle, page_start, SEEK_SET);
		write(sim->handle, data,data_length);
	}

	if(spare) {
		lseek(sim->handle, page_start + sim->chunk_size, SEEK_SET);
		write(sim->handle, spare, spare_length);
	}

	return YAFFS_OK;
}


static int yflex_erase(struct yaffs_dev *dev, int block_id)
{
	struct sim_data *sim = dev_to_sim(dev);

	return yflex_erase_internal(sim, block_id);
}

static int yflex_check_block_bad(struct yaffs_dev *dev, int block_id)
{
	(void) dev;
	(void) block_id;

	return YAFFS_OK;
}

static int yflex_mark_block_bad(struct yaffs_dev *dev, int block_id)
{
	(void) dev;
	(void) block_id;

	return YAFFS_OK;
}

static int yflex_sim_init(struct sim_data *sim)
{
	int h;
	uint32_t fsize = 0;
	uint32_t i;

	sim->total_chunk_size = sim->chunk_size + sim->spare_size;
	sim->total_bytes_per_block = sim->total_chunk_size * sim->chunks_per_block;

	sim->buffer = malloc(sim->total_bytes_per_block);

	h = open(sim->file_name, O_RDWR);
	if (h >= 0) {
		fsize = lseek(h, 0, SEEK_END);
		lseek(h, 0, SEEK_SET);
	}

	if (fsize != sim->total_bytes_per_block * sim->n_blocks) {
		/* Need to create the file. */
		close(h);
		unlink(sim->file_name);
		h = open(sim->file_name, O_RDWR | O_CREAT | O_TRUNC, 0666);
		sim->handle = h;

		for (i = 0; i < sim->n_blocks; i++)
			yflex_erase_internal(sim, i);
	} else {
		sim->handle = h;
	}

	return YAFFS_OK;
}


struct yaffs_dev *yaffs_flexible_file_sim_create(
				const char *name,
				const char *sim_file_name,
				uint32_t n_blocks,
				uint32_t start_block, uint32_t end_block,
				uint32_t chunks_per_block,
				uint32_t bytes_per_chunk,
				uint32_t bytes_per_spare)
{
	struct sim_data *sim;
	struct yaffs_dev *dev;
	struct yaffs_param *p;
	struct yaffs_driver *d;

	sim = malloc(sizeof(*sim));
	dev = malloc(sizeof(*dev));

	if(!sim || !dev){
		free(sim);
		free(dev);
		return NULL;
	}
	memset(sim, 0, sizeof(*sim));
	memset(dev, 0, sizeof(*dev));

	/* Set up sim */
	sim->file_name = strdup(sim_file_name);
	sim->chunks_per_block = chunks_per_block;
	sim->chunk_size = bytes_per_chunk;
	sim->spare_size = bytes_per_spare;
	sim->n_blocks = n_blocks;

	dev->driver_context= (void *)sim;

	yflex_sim_init(sim);



	if(start_block >= sim->n_blocks)
		start_block = 0;
	if(end_block == 0 || end_block >= sim->n_blocks)
		end_block = sim->n_blocks - 1;

	p = &dev->param;
	p->name = strdup(name);
	p->start_block = start_block;
	p->end_block = end_block;
	p->total_bytes_per_chunk = bytes_per_chunk;
	p->spare_bytes_per_chunk = bytes_per_spare;
	p->chunks_per_block = chunks_per_block;
	p->n_reserved_blocks = 2;
	p->use_nand_ecc = 1;
	p->inband_tags = 0;
	p->is_yaffs2 = 1;
	p->n_caches = 10;

	d= &dev->drv;
	d->drv_initialise_fn = yflex_initialise;
	d->drv_deinitialise_fn = yflex_deinitialise;
	d->drv_read_chunk_fn = yflex_rd_chunk;
	d->drv_write_chunk_fn = yflex_wr_chunk;
	d->drv_erase_fn = yflex_erase;
	d->drv_check_bad_fn = yflex_check_block_bad;
	d->drv_mark_bad_fn = yflex_mark_block_bad;

	yaffs_add_device(dev);

	return dev;
}
