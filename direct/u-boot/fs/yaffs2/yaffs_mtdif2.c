/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* mtd interface for YAFFS2 */

/* XXX U-BOOT XXX */
#include <common.h>
#include "asm/errno.h"

#include "yportenv.h"
#include "yaffs_trace.h"

#include "yaffs_mtdif2.h"

#include "linux/mtd/mtd.h"
#include "linux/types.h"
#include "linux/time.h"

#include "yaffs_trace.h"

#include "yaffs_packedtags2.h"
#include "string.h"


int nandmtd2_WriteChunkWithTagsToNAND(struct yaffs_dev* dev, int chunkInNAND,
				      const u8 * data,
				      const struct yaffs_ext_tags * tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	struct mtd_oob_ops ops;
#else
	size_t dummy;
#endif
	int retval = 0;

	loff_t addr = ((loff_t) chunkInNAND) * dev->data_bytes_per_chunk;

	struct yaffs_packed_tags2 pt;

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_WriteChunkWithTagsToNAND chunk %d data %p tags %p",
		chunkInNAND, data, tags);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	if (tags)
		yaffs_pack_tags2(&pt, tags, !dev->param.no_tags_ecc);
	else
		BUG(); /* both tags and data should always be present */

	if (data) {
		ops.mode = MTD_OOB_AUTO;
		ops.ooblen = sizeof(pt);
		ops.len = dev->data_bytes_per_chunk;
		ops.ooboffs = 0;
		ops.datbuf = (u8 *)data;
		ops.oobbuf = (void *)&pt;
		retval = mtd->write_oob(mtd, addr, &ops);
	} else
		BUG(); /* both tags and data should always be present */
#else
	if (tags) {
		yaffs_pack_tags2(&pt, tags);
	}

	if (data && tags) {
		if (dev->param.use_nand_ecc)
			retval =
			    mtd->write_ecc(mtd, addr, dev->data_bytes_per_chunk,
					   &dummy, data, (u8 *) & pt, NULL);
		else
			retval =
			    mtd->write_ecc(mtd, addr, dev->data_bytes_per_chunk,
					   &dummy, data, (u8 *) & pt, NULL);
	} else {
		if (data)
			retval =
			    mtd->write(mtd, addr, dev->data_bytes_per_chunk, &dummy,
				       data);
		if (tags)
			retval =
			    mtd->write_oob(mtd, addr, mtd->oobsize, &dummy,
					   (u8 *) & pt);

	}
#endif

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_ReadChunkWithTagsFromNAND(struct yaffs_dev * dev, int chunkInNAND,
				       u8 * data, struct yaffs_ext_tags * tags)
{
	static u8 *spare_buffer = NULL;

	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	struct mtd_oob_ops ops;
#endif
	size_t dummy;
	int retval = 0;

	loff_t addr = ((loff_t) chunkInNAND) * dev->data_bytes_per_chunk;

	struct yaffs_packed_tags2 pt;
	
	if(!spare_buffer)
		spare_buffer = kmalloc(mtd->oobsize, GFP_NOFS);

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_ReadChunkWithTagsFromNAND chunk %d data %p tags %p",
		chunkInNAND, data, tags);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
	if (data && !tags)
		retval = mtd->read(mtd, addr, dev->data_bytes_per_chunk,
				&dummy, data);
	else if (tags) {
		ops.mode = MTD_OOB_AUTO;
		ops.ooblen = sizeof(pt);
		ops.len = data ? dev->data_bytes_per_chunk : sizeof(pt);
		ops.ooboffs = 0;
		ops.datbuf = data;
		ops.oobbuf = spare_buffer;
		retval = mtd->read_oob(mtd, addr, &ops);
	}
#else
	if (data && tags) {
		if (dev->useNANDECC) {
			retval =
			    mtd->read_ecc(mtd, addr, dev->data_bytes_per_chunk,
					  &dummy, data, dev->spare_buffer,
					  NULL);
		} else {
			retval =
			    mtd->read_ecc(mtd, addr, dev->data_bytes_per_chunk,
					  &dummy, data, dev->spare_buffer,
					  NULL);
		}
	} else {
		if (data)
			retval =
			    mtd->read(mtd, addr, dev->data_bytes_per_chunk, &dummy,
				      data);
		if (tags)
			retval =
			    mtd->read_oob(mtd, addr, mtd->oobsize, &dummy,
					  dev->spare_buffer);
	}
#endif

	memcpy(&pt, spare_buffer, sizeof(pt));

	if (tags)
		yaffs_unpack_tags2(tags, &pt, !dev->param.no_tags_ecc);

	if(tags && retval == -EBADMSG && tags->ecc_result == YAFFS_ECC_RESULT_NO_ERROR)
		tags->ecc_result = YAFFS_ECC_RESULT_UNFIXED;

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int nandmtd2_MarkNANDBlockBad(struct yaffs_dev *dev, int blockNo)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
	int retval;

	yaffs_trace(YAFFS_TRACE_MTD,
		"nandmtd2_MarkNANDBlockBad %d", blockNo);

	retval =
	    mtd->block_markbad(mtd,
			       blockNo * dev->param.chunks_per_block *
			       dev->data_bytes_per_chunk);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;

}

int nandmtd2_QueryNANDBlock(struct yaffs_dev *dev, int blockNo,
			    enum yaffs_block_state * state, int *sequenceNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->driver_context);
	int retval;

	yaffs_trace(YAFFS_TRACE_MTD, "nandmtd2_QueryNANDBlock %d", blockNo);
	retval =
	    mtd->block_isbad(mtd,
			     blockNo * dev->param.chunks_per_block *
			     dev->data_bytes_per_chunk);

	if (retval) {
		yaffs_trace(YAFFS_TRACE_MTD, "block is bad");

		*state = YAFFS_BLOCK_STATE_DEAD;
		*sequenceNumber = 0;
	} else {
		struct yaffs_ext_tags t;
		nandmtd2_ReadChunkWithTagsFromNAND(dev,
						   blockNo *
						   dev->param.chunks_per_block, NULL,
						   &t);

		if (t.chunk_used) {
			*sequenceNumber = t.seq_number;
			*state = YAFFS_BLOCK_STATE_NEEDS_SCAN;
		} else {
			*sequenceNumber = 0;
			*state = YAFFS_BLOCK_STATE_EMPTY;
		}
	}
	yaffs_trace(YAFFS_TRACE_MTD, "block is bad seq %d state %d", *sequenceNumber, *state);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}
