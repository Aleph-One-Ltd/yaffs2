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
#include "yaffs_encryption.h"

#include "yaffs_encrypt_mbedtls.h"

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

static void do_install_null_encryptor(struct yaffs_dev *dev)
{
	/*
	 * For the null encryption, null out dev->enc and
	 * yaffs_enc_init(dev) will sort it out.
	 */
	memset(&dev->enc, 0, sizeof(struct yaffs_encryptor));
}


static void show_dev_info(struct yaffs_dev *dev)
{
	if(!dev)
		return;


	printf("\ttags............. %s\n", dev->param.inband_tags ?
							"inband" : "oob");
	printf("\tencryptor........ %s\n", dev->enc.enc_name);
	printf("\ttotal data bytes. %d\n", dev->param.total_bytes_per_chunk);

	printf("\tencryptor bytes.. %d\n", dev->enc.enc_reserved_bytes);
	printf("\ttagger bytes..... %d\n", dev->tagger.tagger_reserved_bytes);
	printf("\tpayload bytes.... %d\n", dev->payload_bytes_per_chunk);
	printf("\tdata bytes....... %d\n", dev->data_bytes_per_chunk);
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
	else
		do_install_null_encryptor(dev);

	dev->payload_bytes_per_chunk = dev->param.total_bytes_per_chunk -
				       dev->enc.enc_reserved_bytes;

	/*
	 * Note although we're calculating the data_bytes_per_chunk
	 * the tagger is not installed, so this is meaningless.
	 * At this stage we're just testing the payload encryption anyway.
	 */
	dev->data_bytes_per_chunk = dev->payload_bytes_per_chunk -
				       dev->tagger.tagger_reserved_bytes;

	yaffs_encryption_init(dev);
}

static struct yaffs_dev *test_dev_create(int data_area_size, int inband_tags)
{
	struct yaffs_dev *dev = malloc(sizeof(*dev));

	if (!dev)
		return NULL;

	memset(dev, 0, sizeof(*dev));

	dev->param.name = "Encryptor test device";
	dev->param.total_bytes_per_chunk = data_area_size;
	dev->param.inband_tags = inband_tags;
	dev->param.spare_bytes_per_chunk = 0;
	dev->param.chunks_per_block = 32;
	dev->param.start_block = 0;
	dev->param.end_block = 20;
	dev->param.is_yaffs2 = 1;

	if (inband_tags)
		dev->tagger.tagger_reserved_bytes = 16;

	return dev;
}


/*
 *  Set up a tester...
 */
struct yaffs_tester {
	struct yaffs_dev *dev;
};

static struct yaffs_tester *tester_create(struct yaffs_dev *dev)
{
	struct yaffs_tester *tester;

	tester = malloc(sizeof(struct yaffs_tester));
	if (!tester)
		return NULL;

	memset(tester, 0, sizeof(*tester));

	tester->dev = dev;

#if 0
	tester->buffer_size = dev->param.total_bytes_per_chunk;

	for(i = 0; i < N_TEST_BUFFERS; i++) {
		tester->buffer[i] = malloc(tester->buffer_size);
		if (!tester->buffer[i])
			goto buffer_alloc_failed;
		memset(tester->buffer[i], 0, tester->buffer_size);
	}
#endif
	return tester;

#if 0

buffer_alloc_failed:

	for(i = 0; i < N_TEST_BUFFERS; i++)
		free(tester->buffer[i]);
#endif

	free(tester);
	return NULL;
}

/*
 *  Tests return negative number if there is a problem.
 *  Return 0 for success.
 */

static  int test_empty_chunk_read(struct yaffs_tester *tester)
{
	(void) tester;

	return 0;
}

static int do_tests(struct yaffs_tester *tester)
{
	int ret;

	/*
	 * The fake device is initialised as erased.
	 * We're just reading a chunk into a buffer to check
	 * that the reading of an 0xFF chunk works properly.
	 */

	ret = test_empty_chunk_read(tester);

	if(ret < 0) {

	}

	return 0;
}

int encryptor_test(int data_area_size,
		   int inband_tags,
		   const char *selected_encryptor)
{
	struct yaffs_dev *dev;
	struct yaffs_tester *tester;
	int ret;

	dev = test_dev_create(data_area_size,
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

