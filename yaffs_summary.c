/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Summaries write the useful part of the tags for the chunks in a block into an
 * an array which is written to the last n chunks of the block.
 * Reading the summaries gives all the tags for the block in one read. Much
 * faster.
 *
 * Chunks holding summaries are marked with tags making it look like
 * they are part of a fake file.
 *
 * The summary could also be used during gc.
 *
 */

#include "yaffs_summary.h"
#include "yaffs_packedtags2.h"
#include "yaffs_nand.h"
#include "yaffs_getblockinfo.h"
#include "yaffs_bitmap.h"

/* Summary tags don't need the sequence number because that is redundant. */
struct yaffs_summary_tags {
	unsigned obj_id;
	unsigned chunk_id;
	unsigned n_bytes;
};

static void yaffs_summary_clear(struct yaffs_dev *dev)
{
	if(!dev->sum_tags)
		return;
	memset(dev->sum_tags, 0, dev->chunks_per_summary *
		sizeof(struct yaffs_summary_tags));
}

int yaffs_summary_init(struct yaffs_dev *dev)
{
	int sum_bytes;
	int chunks_used; /* Number of chunks used by summary */

	sum_bytes = dev->param.chunks_per_block *
			sizeof(struct yaffs_summary_tags);

	chunks_used = (sum_bytes + dev->data_bytes_per_chunk - 1)/
			dev->data_bytes_per_chunk;
	dev->chunks_per_summary = dev->param.chunks_per_block - chunks_used;
	dev->sum_tags = kmalloc(sizeof(struct yaffs_summary_tags) *
				dev->chunks_per_summary, GFP_NOFS);
	dev->gc_sum_tags = kmalloc(sizeof(struct yaffs_summary_tags) *
				dev->chunks_per_summary, GFP_NOFS);
	if(!dev->sum_tags || !dev->gc_sum_tags) {
		kfree(dev->sum_tags);
		kfree(dev->gc_sum_tags);
		return YAFFS_FAIL;
	}

	yaffs_summary_clear(dev);

	return YAFFS_OK;
}

void yaffs_summary_deinit(struct yaffs_dev *dev)
{
	kfree(dev->sum_tags);
	dev->sum_tags = NULL;
	kfree(dev->gc_sum_tags);
	dev->gc_sum_tags = NULL;
	dev->chunks_per_summary = 0;
}

static int yaffs_summary_write(struct yaffs_dev *dev, int blk)
{
	struct yaffs_ext_tags tags;
	u8 *buffer;
	u8 *sum_buffer = (u8 *)dev->sum_tags;
	int n_bytes;
	int chunk_in_nand;
	int chunk_in_block;
	int result;
	int this_tx;
	struct yaffs_block_info *bi = yaffs_get_block_info(dev, blk);

	buffer = yaffs_get_temp_buffer(dev);
	n_bytes = sizeof(struct yaffs_summary_tags) * dev->chunks_per_summary;
	memset(&tags, 0, sizeof(struct yaffs_ext_tags));
	tags.obj_id = YAFFS_OBJECTID_SUMMARY;
	tags.chunk_id = 1;
	chunk_in_block = dev->chunks_per_summary;
	chunk_in_nand = dev->alloc_block * dev->param.chunks_per_block +
						dev-> chunks_per_summary;
	do {
		this_tx = n_bytes;
		if (this_tx > dev->data_bytes_per_chunk)
			this_tx = dev->data_bytes_per_chunk;
		memcpy(buffer, sum_buffer, this_tx);
		tags.n_bytes = this_tx;
		result = yaffs_wr_chunk_tags_nand(dev, chunk_in_nand,
						buffer, &tags);

		if (result != YAFFS_OK)
			break;
		yaffs_set_chunk_bit(dev, blk, chunk_in_block);
		bi->pages_in_use++;
		dev->n_free_chunks--;

		n_bytes -= this_tx;
		sum_buffer += this_tx;
		chunk_in_nand++;
		chunk_in_block++;
		tags.chunk_id++;
	} while (result == YAFFS_OK && n_bytes > 0);
	yaffs_release_temp_buffer(dev, buffer);


	if (result == YAFFS_OK)
		bi->has_summary = 1;


	return result;
}

int yaffs_summary_read(struct yaffs_dev *dev,
			struct yaffs_summary_tags *st,
			int blk)
{
	struct yaffs_ext_tags tags;
	u8 *buffer;
	u8 *sum_buffer = (u8 *)st;
	int n_bytes;
	int chunk_id;
	int chunk_in_nand;
	int chunk_in_block;
	int result;
	int this_tx;
	struct yaffs_block_info *bi = yaffs_get_block_info(dev, blk);

	buffer = yaffs_get_temp_buffer(dev);
	n_bytes = sizeof(struct yaffs_summary_tags) * dev->chunks_per_summary;
	chunk_in_block = dev->chunks_per_summary;
	chunk_in_nand = blk * dev->param.chunks_per_block +
							dev->chunks_per_summary;
	chunk_id = 1;
	do {
		this_tx = n_bytes;
		if(this_tx > dev->data_bytes_per_chunk)
			this_tx = dev->data_bytes_per_chunk;
		result = yaffs_rd_chunk_tags_nand(dev, chunk_in_nand,
						buffer, &tags);

		if (tags.chunk_id != chunk_id ||
			tags.obj_id != YAFFS_OBJECTID_SUMMARY ||
			tags.chunk_used == 0 ||
			tags.ecc_result > YAFFS_ECC_RESULT_FIXED ||
			this_tx != tags.n_bytes)
				result = YAFFS_FAIL;
		if (result != YAFFS_OK)
			break;

		if (st == dev->sum_tags) {
			/* If we're scanning then update the block info */
			yaffs_set_chunk_bit(dev, blk, chunk_in_block);
			bi->pages_in_use++;
		}

		memcpy(sum_buffer, buffer, this_tx);
		n_bytes -= this_tx;
		sum_buffer += this_tx;
		chunk_in_nand++;
		chunk_in_block++;
		chunk_id++;
	} while (result == YAFFS_OK && n_bytes > 0);
	yaffs_release_temp_buffer(dev, buffer);

	if (st == dev->sum_tags && result == YAFFS_OK)
		bi->has_summary = 1;

	return result;
}
int yaffs_summary_add(struct yaffs_dev *dev,
			struct yaffs_ext_tags *tags,
			int chunk_in_nand)
{
	struct yaffs_packed_tags2_tags_only tags_only;
	struct yaffs_summary_tags *sum_tags;
	int block_in_nand = chunk_in_nand / dev->param.chunks_per_block;
	int chunk_in_block = chunk_in_nand % dev->param.chunks_per_block;

	if(!dev->sum_tags)
		return YAFFS_OK;

	if(chunk_in_block >= 0 && chunk_in_block < dev->chunks_per_summary) {
		yaffs_pack_tags2_tags_only(&tags_only, tags);
		sum_tags = &dev->sum_tags[chunk_in_block];
		sum_tags->chunk_id = tags_only.chunk_id;
		sum_tags->n_bytes = tags_only.n_bytes;
		sum_tags->obj_id = tags_only.obj_id;

		if(chunk_in_block == dev->chunks_per_summary - 1) {
			/* Time to write out the summary */
			yaffs_summary_write(dev, block_in_nand);
			yaffs_summary_clear(dev);
			yaffs_skip_rest_of_block(dev);
		}
	}
	return YAFFS_OK;
}

int yaffs_summary_fetch(struct yaffs_dev *dev,
			struct yaffs_ext_tags *tags,
			int chunk_in_block)
{
	struct yaffs_packed_tags2_tags_only tags_only;
	struct yaffs_summary_tags *sum_tags;
	if(chunk_in_block >= 0 && chunk_in_block < dev->chunks_per_summary) {
		sum_tags = &dev->sum_tags[chunk_in_block];
		tags_only.chunk_id = sum_tags->chunk_id;
		tags_only.n_bytes = sum_tags->n_bytes;
		tags_only.obj_id = sum_tags->obj_id;
		yaffs_unpack_tags2_tags_only(tags, &tags_only);
		return YAFFS_OK;
	}
	return YAFFS_FAIL;
}

void yaffs_summary_gc(struct yaffs_dev *dev, int blk)
{
	struct yaffs_block_info *bi = yaffs_get_block_info(dev, blk);
	int i;

	if (!bi->has_summary)
		return;

	for (i = dev->chunks_per_summary; i < dev->param.chunks_per_block; i++) {
		if( yaffs_check_chunk_bit(dev, blk, i)) {
			yaffs_clear_chunk_bit(dev, blk, i);
			bi->pages_in_use--;
			dev->n_free_chunks++;
		}
	}

}
