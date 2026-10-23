/*
 *
 */

#ifndef __FAKE_DEV_H__
#define __FAKE_DEV_H__

struct yaffs_dev;

struct yaffs_dev *fake_dev_create(int n_blocks,
				  int chunks_per_block,
				  int data_area_size,
				  int spare_area_size,
				  int is_yaffs2,
				  int inband_tags);

#endif
