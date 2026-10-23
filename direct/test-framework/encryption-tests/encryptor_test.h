#ifndef __ENCRYPTOR_TEST_H__
#define __ENCRYPTOR_TEST_H__

struct yaffs_dev;

int encryptor_test(int total_bytes_per_chunk,
		   int inband_tags,
		   const char * selected_encryptor);

#endif
