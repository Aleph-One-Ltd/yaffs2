/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This is a fake device that is part of a test harness for a Yaffs encryptor.
 * This allows the encryptor to be tested without the whole of Yaffs.
 *
 * This works by setting up a very stripped down yaffs_dev that has a
 * fake driver layer and can prod and observe the encryptor.
 */

#include "fake_dev.h"
#include "yaffs_guts.h"

#include <string.h>


#define dev2context(dev) ((struct local_context *)(dev->driver_context))

struct local_context {
	u8 *store;
	u8 *bbm;
	int n_blocks;
	int n_chunks;
	int n_data_bytes;
	int n_spare_bytes;
};

static int store_chunk_size(struct yaffs_dev *dev)
{
	return dev->param.total_bytes_per_chunk +
	       dev->param.spare_bytes_per_chunk;
}

static int store_block_size(struct yaffs_dev *dev)
{
	return store_chunk_size(dev) *
	       dev->param.chunks_per_block;
}

static int store_nblocks(struct yaffs_dev *dev)
{
	return dev->param.end_block + 1;
}

static int store_nchunks(struct yaffs_dev *dev)
{
	return dev->param.chunks_per_block * store_nblocks(dev);
}

static int store_erase_block(struct yaffs_dev *dev, int block)
{
	struct local_context *context = dev2context(dev);
	u8 *store = context->store;
	int block_size = store_block_size(dev);

	memset(store + block * block_size, 0xff, block_size);

	return YAFFS_OK;
}


static int store_create(struct yaffs_dev *dev)
{
	struct local_context *context = malloc(sizeof *context);
	int i;

	if (!context)
		return YAFFS_FAIL;

	memset(context, 0, sizeof(*context));

	context->n_blocks = store_nblocks(dev);
	context->n_chunks = store_nchunks(dev);
	context->n_data_bytes = dev->param.total_bytes_per_chunk;
	context->n_spare_bytes = dev->param.spare_bytes_per_chunk;
	context->store = malloc(store_block_size(dev) * context->n_blocks);
	context->bbm = malloc(context->n_blocks);

	if (!context->store || !context->bbm)
		goto fail;

	dev->driver_context = context;

	for (i = 0; i < context->n_blocks; i++)
		store_erase_block(dev, i);

	memset(context->bbm, 0, context->n_blocks);

	return YAFFS_OK;
fail:
	free(context->store);
	free(context);
	return YAFFS_FAIL;
}

static int fake_dev_check_bad(struct yaffs_dev *dev, int block_no)
{
	struct local_context *context = dev2context(dev);

	if (block_no >= 0 && block_no < context->n_blocks)
		return (context->bbm[block_no] == 0) ? YAFFS_OK : YAFFS_FAIL;
	return YAFFS_FAIL;
}

static int fake_dev_mark_bad(struct yaffs_dev *dev, int block_no)
{
	struct local_context *context = dev2context(dev);

	if (block_no >= 0 && block_no < context->n_blocks) {
		context->bbm[block_no] = 1;
		return YAFFS_OK;
	}
	return YAFFS_FAIL;
}

static int fake_dev_erase(struct yaffs_dev *dev, int block_no)
{
	struct local_context *context = dev2context(dev);

	if (block_no >= 0 && block_no < context->n_blocks) {
		store_erase_block(dev, block_no);
		return YAFFS_OK;
	}
	return YAFFS_FAIL;
}

static int fake_dev_initialise(struct yaffs_dev *dev)
{
	(void) dev;
	return YAFFS_OK;
}

static int fake_dev_deinitialise(struct yaffs_dev *dev)
{
	(void) dev;
	return YAFFS_OK;
}

static int fake_dev_read_chunk(struct yaffs_dev *dev, int nand_chunk,
                                   u8 *data, int data_len,
                                   u8 *oob, int oob_len,
                                   enum yaffs_ecc_result *ecc_result)
{
	struct local_context *context = dev2context(dev);
	u8 *chunk_base;

	if (nand_chunk < 0 || nand_chunk >= context->n_chunks)
		return YAFFS_FAIL;

	chunk_base = context->store +
		nand_chunk * (context->n_data_bytes + context->n_spare_bytes);
	if (data)
		memcpy(data, chunk_base, data_len);
	if (oob)
		memcpy(oob, chunk_base + context->n_data_bytes, oob_len);

	*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	return YAFFS_OK;
}

static int fake_dev_write_chunk(struct yaffs_dev *dev, int nand_chunk,
                                   const u8 *data, int data_len,
                                   const u8 *oob, int oob_len)
{
	struct local_context *context = dev2context(dev);
	u8 *chunk_base;

	if (nand_chunk < 0 || nand_chunk >= context->n_chunks)
		return YAFFS_FAIL;

	chunk_base = context->store +
		nand_chunk * (context->n_data_bytes + context->n_spare_bytes);
	if (data)
		memcpy(chunk_base, data, data_len);
	if (oob)
		memcpy(chunk_base + context->n_data_bytes, oob, oob_len);

	return YAFFS_OK;
}


struct yaffs_dev *fake_dev_create(int n_blocks,
				  int chunks_per_block,
				  int data_area_size,
				  int spare_area_size,
				  int is_yaffs2,
				  int inband_tags)
{
	struct yaffs_dev *dev;

	dev = malloc(sizeof(*dev));

	if (!dev)
		return dev;

	memset(dev, 0, sizeof(*dev));
	dev->param.name = "Encryptor test device";
	dev->param.total_bytes_per_chunk = data_area_size;
	dev->param.spare_bytes_per_chunk = spare_area_size;
	dev->param.chunks_per_block = chunks_per_block;
	dev->param.start_block = 0;
	dev->param.end_block = n_blocks - 1;
	dev->param.is_yaffs2 = is_yaffs2;
	dev->param.inband_tags = inband_tags;

	/*
	 * Set up driver.
	 */
	dev->drv.drv_check_bad_fn    = fake_dev_check_bad;
	dev->drv.drv_mark_bad_fn     = fake_dev_mark_bad;
	dev->drv.drv_deinitialise_fn = fake_dev_deinitialise;
	dev->drv.drv_initialise_fn   = fake_dev_initialise;
	dev->drv.drv_erase_fn	     = fake_dev_erase;
	dev->drv.drv_write_chunk_fn  = fake_dev_write_chunk;
	dev->drv.drv_read_chunk_fn   = fake_dev_read_chunk;


	store_create(dev);

	return dev;
}
