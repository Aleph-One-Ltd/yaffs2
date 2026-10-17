#ifndef __ENCRYPTOR_TEST_H__
#define __ENCRYPTOR_TEST_H__

struct yaffs_dev;

int encryptor_test(int n_blocks,
		   int chunks_per_block,
		   int data_area_size,
		   int spare_area_size,
		   int is_yaffs2,
		   int inband_tags,
		   const char * selected_encryptor);

#endif
