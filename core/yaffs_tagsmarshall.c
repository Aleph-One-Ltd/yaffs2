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
 *
 * This file handles the marshalling (ie internal<-->external structure
 * translation between the internal tags and the stored tags in yaffs2-style
 * tags storage.
 * See yaffs_tagscompat.c for yaffs1 style flash layouts.
 */

#include "yaffs_tagsmarshall.h"
#include "yaffs_trace.h"
#include "yaffs_packedtags2.h"
#include "yaffs_encryption.h"

/*
 * For writing, we are going to set up the data part of the chunk, then
 * encrypt is and write to flash.
 * For inband tags the tags are written into the data part of the chunk, for
 * oob tags the tags are wrapped in ECC and written to the spare area.
 */
static int yaffs_tags_marshall_write(struct yaffs_dev *dev,
				    int nand_chunk, const u8 *data,
				    const struct yaffs_ext_tags *tags)
{
	struct yaffs_packed_tags2 pt;
	int retval;
	const u8 *transfer_buffer;

	int packed_tags_size =
	    dev->param.no_tags_ecc ? sizeof(pt.t) : sizeof(pt);
	void *packed_tags_ptr =
	    dev->param.no_tags_ecc ? (void *)&pt.t : (void *)&pt;

	yaffs_trace(YAFFS_TRACE_MTD,
		"yaffs_tags_marshall_write chunk %d data %p tags %p",
		nand_chunk, data, tags);

	/* For yaffs2 writing there must be both data and tags.
	 * If we're using inband tags, then the tags are stuffed into
	 * the end of the data buffer.
	 */

	transfer_buffer = yaffs_encryption_get_write_buffer(dev, data);

	if (!data || !tags)
		BUG();
	else if (dev->param.inband_tags) {
		struct yaffs_packed_tags2_tags_only *pt2tp;
		pt2tp =
		    (struct yaffs_packed_tags2_tags_only *)
				(data + dev->data_bytes_per_chunk);

		yaffs_pack_tags2_tags_only(dev, pt2tp, tags);

		/* The encryption step places data into the transfer buffer. */
		retval = yaffs_encryption_encrypt(dev,
					data, dev->payload_bytes_per_chunk);
		retval = dev->drv.drv_write_chunk_fn(dev, nand_chunk,
			transfer_buffer, dev->param.total_bytes_per_chunk,
			NULL, 0);
	} else {
		yaffs_pack_tags2(dev, &pt, tags, !dev->param.no_tags_ecc);

		/* The encryption step places data into the transfer buffer. */
		retval = yaffs_encryption_encrypt(dev,
					data, dev->payload_bytes_per_chunk);
		retval = dev->drv.drv_write_chunk_fn(dev, nand_chunk,
			transfer_buffer, dev->param.total_bytes_per_chunk,
			packed_tags_ptr, packed_tags_size);
	}

	return retval;
}

/*
 * For reading, we first read into the transfer buffer. The decryption will
 * move data into the data buffer where the inband tags can be extracted if
 * need be.
 */

static int yaffs_tags_marshall_read(struct yaffs_dev *dev,
				   int nand_chunk, u8 *data,
				   struct yaffs_ext_tags *tags)
{
	int retval = 0;
	int ret_enc = 0;
	int local_data = 0;
	u8 spare_buffer[100];
	enum yaffs_ecc_result ecc_result;
	u8 *transfer_buffer;
	struct yaffs_packed_tags2 pt;
	int read_spare;

	int packed_tags_size =
	    dev->param.no_tags_ecc ? sizeof(pt.t) : sizeof(pt);
	void *packed_tags_ptr =
	    dev->param.no_tags_ecc ? (void *)&pt.t : (void *)&pt;

	yaffs_trace(YAFFS_TRACE_MTD,
		"yaffs_tags_marshall_read chunk %d data %p tags %p",
		nand_chunk, data, tags);

	if (dev->param.inband_tags) {
		if (!data) {
			local_data = 1;
			data = yaffs_get_temp_buffer(dev);
		}
	}

	if (dev->param.inband_tags)
		read_spare = 0;
	else if (data && !tags)
		read_spare = 0;
	else if (tags)
		read_spare = 1;
	else
		BUG();

	transfer_buffer = yaffs_encryption_get_read_buffer(dev, data);

	retval = dev->drv.drv_read_chunk_fn(dev, nand_chunk,
					transfer_buffer,
					dev->param.total_bytes_per_chunk,
					(read_spare)? spare_buffer : NULL,
					(read_spare)? packed_tags_size : 0,
					&ecc_result);

	/* Note encryption functions should return 0. */
	ret_enc = yaffs_encryption_decrypt(dev,
					   data, dev->payload_bytes_per_chunk);
	if (ret_enc != 0)
		retval = YAFFS_FAIL;

	if (retval == YAFFS_FAIL) {
		if (local_data)
			yaffs_release_temp_buffer(dev, data);

		return YAFFS_FAIL;
	}

	if (dev->param.inband_tags) {
		if (tags) {
			struct yaffs_packed_tags2_tags_only *pt2tp;
			pt2tp =
				(struct yaffs_packed_tags2_tags_only *)
				&data[dev->data_bytes_per_chunk];
			yaffs_unpack_tags2_tags_only(dev, tags, pt2tp);
		}
	} else if (tags) {
		memcpy(packed_tags_ptr, spare_buffer, packed_tags_size);
		yaffs_unpack_tags2(dev, tags, &pt, !dev->param.no_tags_ecc);
	}

	if (local_data)
		yaffs_release_temp_buffer(dev, data);

	if (tags && ecc_result == YAFFS_ECC_RESULT_UNFIXED) {
		tags->ecc_result = YAFFS_ECC_RESULT_UNFIXED;
		dev->n_ecc_unfixed++;
	}

	if (tags && ecc_result == YAFFS_ECC_RESULT_FIXED) {
		if (tags->ecc_result <= YAFFS_ECC_RESULT_NO_ERROR)
			tags->ecc_result = YAFFS_ECC_RESULT_FIXED;
		dev->n_ecc_fixed++;
	}

	if (ecc_result < YAFFS_ECC_RESULT_UNFIXED)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

static int yaffs_tags_marshall_query_block(struct yaffs_dev *dev, int block_no,
			       enum yaffs_block_state *state,
			       u32 *seq_number)
{
	int retval;

	yaffs_trace(YAFFS_TRACE_MTD, "yaffs_tags_marshall_query_block %d",
			block_no);

	retval = dev->drv.drv_check_bad_fn(dev, block_no);

	if (retval== YAFFS_FAIL) {
		yaffs_trace(YAFFS_TRACE_MTD, "block is bad");

		*state = YAFFS_BLOCK_STATE_DEAD;
		*seq_number = 0;
	} else {
		struct yaffs_ext_tags t;

		yaffs_tags_marshall_read(dev,
				    block_no * dev->param.chunks_per_block,
				    NULL, &t);

		if (t.chunk_used) {
			*seq_number = t.seq_number;
			*state = YAFFS_BLOCK_STATE_NEEDS_SCAN;
		} else {
			*seq_number = 0;
			*state = YAFFS_BLOCK_STATE_EMPTY;
		}
	}

	yaffs_trace(YAFFS_TRACE_MTD,
		"block query returns  seq %d state %d",
		*seq_number, *state);

	return retval;
}

static int yaffs_tags_marshall_mark_bad(struct yaffs_dev *dev, int block_no)
{
	return dev->drv.drv_mark_bad_fn(dev, block_no);
}


void yaffs_tags_marshall_install(struct yaffs_dev *dev)
{
	if (!dev->param.is_yaffs2)
		return;

	if (!dev->tagger.write_chunk_tags_fn) {
		dev->tagger.write_chunk_tags_fn = yaffs_tags_marshall_write;
		dev->tagger.read_chunk_tags_fn = yaffs_tags_marshall_read;
		dev->tagger.query_block_fn = yaffs_tags_marshall_query_block;
		dev->tagger.mark_bad_fn = yaffs_tags_marshall_mark_bad;
		dev->tagger.tagger_reserved_bytes =
			sizeof(struct yaffs_packed_tags2_tags_only);
		dev->tagger.tagger_name = "tags_marshall";
	}

}
