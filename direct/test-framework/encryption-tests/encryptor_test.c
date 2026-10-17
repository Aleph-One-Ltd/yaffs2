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
 * This is a test harness for a Yaffs encryptor.
 * This allows the encryptor to be tested without the whole of Yaffs.
 *
 * This works by setting up a very stripped down yaffs_dev that has a
 * fake driver layer and can prod and observe the encryptor.
 */
#include "encryptor_test.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#include "yaffs_guts.h"

#include "yaffs_encrypt_mbedtls.h"
#include "fake_dev.h"

/*
 * Needed by yaffsfs.
 */

void (*ext_fatal)(void);

char mount_point[200];

void yaffsfs_free(void *ptr)
{
	free(ptr);
}

void *yaffsfs_malloc(size_t size)
{
	return malloc(size);
}

/*
 * Encryption initialisation.
 */

static unsigned char key[16] =
			{ 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

static void do_install_mbedtls_encryptor(struct yaffs_dev *dev)
{
	yaffs_encrypt_mbedtls_install(dev, key, sizeof(key));
}


static void show_dev_info(struct yaffs_dev *dev)
{
	if(!dev)
		return;

	printf("\tname............. %s\n", dev->param.name);
	printf("\ttype............. %s\n", dev->param.is_yaffs2 ? "yaffs2" : "yaffs1");
	printf("\ttags............. %s\n", dev->param.inband_tags ? "inband" : "oob");
	printf("\ttotal data bytes. %d\n", dev->param.total_bytes_per_chunk);
	printf("\tspare bytes...... %d\n", dev->param.spare_bytes_per_chunk);
	printf("\tstart block...... %d\n", dev->param.start_block);
	printf("\tend block........ %d\n", dev->param.end_block);
	printf("\tencryptor........ %s\n", dev->enc.enc_name ? dev->enc.enc_name : "none");
	printf("\tencryptor bytes.. %d\n", dev->enc.enc_reserved_bytes);
	printf("\tpayload bytes.... %d\n", dev->payload_bytes_per_chunk);
}

/*
 * prepare_dev() does some of the intiialisation typically performed by
 * yaffs_guts_initialise. Since we are not using the top part of yaffs
 * (just the low level stuff) we just do the bare minimum.
 */

static void prepare_dev(struct yaffs_dev *dev, const char *selected_encryptor)
{
	if (strcmp(selected_encryptor, "mbedtls") == 0)
		do_install_mbedtls_encryptor(dev);

	dev->payload_bytes_per_chunk = dev->param.total_bytes_per_chunk -
				       dev->enc.enc_reserved_bytes;

	/*
	 * Note although we're calculating the data_bytes_per_chunk
	 * the tagger is not isnstalled, so this is meaningless.
	 * At this stage we're just testing the payload encryption anyway.
	 */
	dev->data_bytes_per_chunk = dev->payload_bytes_per_chunk -
				       dev->tagger.tagger_reserved_bytes;
}

/*
 *  Set up a tester...
 */
#define N_TEST_BUFFERS 2
struct yaffs_tester {
	struct yaffs_dev *dev;
	int buffer_size;
	u8 *buffer[N_TEST_BUFFERS];
};

static struct yaffs_tester *tester_create(struct yaffs_dev *dev)
{
	struct yaffs_tester *tester;
	int i;

	tester = malloc(sizeof(struct yaffs_tester));
	if (!tester)
		return NULL;

	tester->dev = dev;
	tester->buffer_size = dev->param.total_bytes_per_chunk;

	for(i = 0; i < N_TEST_BUFFERS; i++) {
		tester->buffer[i] = malloc(tester->buffer_size);
		if (!tester->buffer[i])
			goto buffer_alloc_failed;
		memset(tester->buffer[i], 0, tester->buffer_size);
	}

	return tester;

buffer_alloc_failed:

	for(i = 0; i < N_TEST_BUFFERS; i++)
		free(tester->buffer[i]);
	free(tester);
	return NULL;
}

/*
 *  Tests return negative number if there is a problem.
 *  Return 0 for success.
 */

static  int test_empty_chunk_read(struct yaffs_tester *tester,
				  int buffer_id, int chunk_id)
{
	u8 *buffer;
	struct yaffs_ext_tags tags;
	int ret;

	buffer = tester->buffer[buffer_id];
	memset(buffer, 0, tester->buffer_size);
	ret = yaffs_rd_chunk_tags_nand(tester->dev, chunk_id,
			     buffer, &tags);
	if (ret != YAFFS_OK)
}

static int do_tests(struct yaffs_tester *tester)
{
	int ret;

	/*
	 * The fake device is initialised as erased.
	 * We're just reading a chunk into a buffer to check
	 * that the reading of an 0xFF chunk works properly.
	 */

	ret = test_empty_chunk_read(tester, 0, 0);

	if(ret < 0) {

	}

	return 0;
}

int encryptor_test(int n_blocks,
		   int chunks_per_block,
		   int data_area_size,
		   int spare_area_size,
		   int is_yaffs2,
		   int inband_tags,
		   const char *selected_encryptor)
{
	struct yaffs_dev *dev;
	struct yaffs_tester *tester;
	int ret;

	dev = fake_dev_create(n_blocks,
			      chunks_per_block,
			      data_area_size,
			      spare_area_size,
			      is_yaffs2,
			      inband_tags);
	if (!dev) {
		printf("Could not create test device\n");
		return -ENODEV;
	}

	prepare_dev(dev, selected_encryptor);
	show_dev_info(dev);

	tester = tester_create(dev);

	if(!tester) {
		printf("Could not create yaffs tester\n");
		return -ENODEV;
	}

	ret = do_tests(tester);

	if (ret == 0) {
		printf("All tests ran OK\n");
	} else {
		printf("Some errors in tests\n");
	}

	if (ret)
		return 1;

	return 0;
}

