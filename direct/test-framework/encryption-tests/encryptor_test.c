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
 * This works by setting up a very stripped down yaffs_dev that
 * can prod and observe the encryptor.
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

extern int verbose;

#define dprintf(format, ...) \
	do{ if(verbose) printf(format, ##__VA_ARGS__); } while (0)

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


	printf("Test conditions:\n");
	printf("\ttags............. %s\n", dev->param.inband_tags ?
							"inband" : "oob");
	printf("\tencryptor........ %s\n", dev->enc.enc_name);
	printf("\ttotal data bytes. %d\n", dev->param.total_bytes_per_chunk);

	printf("\tencryptor bytes.. %d\n", yaffs_encryption_get_reserved_bytes(dev));
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
				       yaffs_encryption_get_reserved_bytes(dev);

	/*
	 * Note although we're calculating the data_bytes_per_chunk
	 * the tagger is not installed, so this is meaningless.
	 * At this stage we're just testing the payload encryption anyway.
	 */
	dev->data_bytes_per_chunk = dev->payload_bytes_per_chunk -
				       dev->tagger.tagger_reserved_bytes;

	yaffs_encryption_init(dev);
}

/*
 * We only need a small amount of the yaffs_dev to do the encryptor testing.
 * The device is not mounted and the driver is not needed - just some minimal
 * stuff around the encryptor.
 */
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
#define TESTER_N_BUFFERS 2
struct yaffs_tester {
	struct yaffs_dev *dev;
	int buffer_size;
	u8 *flash_buffer[TESTER_N_BUFFERS];
	u8 *plaintext_buffer[TESTER_N_BUFFERS];
};

static struct yaffs_tester *tester_create(struct yaffs_dev *dev)
{
	struct yaffs_tester *tester;
	int i;

	tester = malloc(sizeof(struct yaffs_tester));
	if (!tester)
		return NULL;

	memset(tester, 0, sizeof(*tester));

	tester->dev = dev;

	tester->buffer_size = dev->param.total_bytes_per_chunk;

	for(i = 0; i < TESTER_N_BUFFERS; i++) {
		tester->flash_buffer[i] = malloc(tester->buffer_size);
		tester->plaintext_buffer[i] = malloc(tester->buffer_size);
		if (!tester->flash_buffer[i] ||
		    !tester->plaintext_buffer[i])
			goto buffer_alloc_failed;
		memset(tester->flash_buffer[i], 0, tester->buffer_size);
		memset(tester->plaintext_buffer[i], 0, tester->buffer_size);
	}
	return tester;

buffer_alloc_failed:

	for(i = 0; i < TESTER_N_BUFFERS; i++) {
		free(tester->flash_buffer[i]);
		free(tester->plaintext_buffer[i]);
	}

	free(tester);
	return NULL;
}

/*
 *  Tests return negative number if there is a problem.
 *  Return 0 for success.
 */


static int test_is_all_ff(const u8 *buffer, int n)
{
	int i;
	dprintf("Checking for %d 0xFF bytes\n", n);
	for (i = 0; i < n; i++) {
		if (buffer[i] != 0xff) {
			dprintf("Check failed at byte %d\n",
				 i);
			return 0;
		}
	}

	dprintf("Check OK\n");
	return 1;
}

/*
 * Testing that the read of an unwritten buffer (all 0xff) results
 * in a buffer that is all 0xff. ie. The encryptor must not modify
 * unwritten pages.
 */
static int test_erased_chunk_read(struct yaffs_tester *tester)
{
	struct yaffs_dev *dev;
	u8 *result_buffer;
	u8 *read_buffer;
	int ret;

	dprintf("test_erased_chunk_read()\n"
		"\tWhen an erased chunk (all 0xFF) is read, this must be passed\n"
		"\tthrough unmodified. The encryptor should not attempt to\n"
		"\tdecrypt the chunk. It must end up still being 0xff bytes so\n"
		"\tthat Yaffs can see it as an erased page.\n"
		);

	dev = tester->dev;
	result_buffer = tester->plaintext_buffer[0];

	memset(result_buffer, 0x55, dev->payload_bytes_per_chunk);

	read_buffer = yaffs_encryption_get_read_buffer(dev, result_buffer);

	if(!read_buffer) {
		printf("Read buffer was NULL!\n");
		return -1;
	}

	dprintf("Copying %d 0xFF bytes to read buffer simulating the read of an erased chunk.\n",
				dev->param.total_bytes_per_chunk);
	memset(read_buffer, 0xff, dev->param.total_bytes_per_chunk);

	ret = yaffs_encryption_decrypt(dev, result_buffer,
				       dev->payload_bytes_per_chunk);
	if (ret != 0) {
		printf("Expecting decryption function to return 0.");
		return -1;
	}

	if (!test_is_all_ff(read_buffer, dev->payload_bytes_per_chunk)) {
		printf("test_empty_chunk_read() failed\n");
		return -1;
	}
	return 0;
}

/*
 * Basic test: set up a plaintext buffer, encrypt it then decrypt it into
 * a different buffer and check both are the same.
 */
static int test_basic_encrypt_decrypt(struct yaffs_tester *tester)
{
	struct yaffs_dev *dev;
	u8 *start_buffer;
	u8 *output_buffer;
	const u8 *write_buffer;
	u8 *read_buffer;
	u32 i;
	int ret;

	dprintf("test_basic_encrypt_decrypt()\n"
		"\tBasic test setting up a plaintext buffer then encrypting it\n"
		"\tand decrypting it into a different buffer and comparing them.\n"
		);

	dev = tester->dev;
	start_buffer = tester->plaintext_buffer[0];
	output_buffer = tester->plaintext_buffer[1];

	for(i = 0; i < dev->payload_bytes_per_chunk; i++) {
		start_buffer[i] = i;
	}

	memset(output_buffer, 0x55, dev->payload_bytes_per_chunk);

	write_buffer = yaffs_encryption_get_write_buffer(dev, start_buffer);

	dprintf("Encrypting start plaintext buffer\n");
	ret = yaffs_encryption_encrypt(dev, start_buffer,
				       dev->payload_bytes_per_chunk);
	if (ret != 0) {
		printf("Expecting encryption function to return 0.");
		return -1;
	}

	dprintf("Copy write data to flash buffer (simulate flash write\n");
	memcpy(tester->flash_buffer[0], write_buffer,
				dev->param.total_bytes_per_chunk);

	/*
	 * Start read simulation and decrypt.
	 */
	read_buffer = yaffs_encryption_get_read_buffer(dev, output_buffer);

	dprintf("Copy flash data to read data (simulate flash read\n");
	memcpy(read_buffer, tester->flash_buffer[0],
			    dev->param.total_bytes_per_chunk);

	dprintf("Decrypt into output plaintext buffer\n");
	ret = yaffs_encryption_decrypt(dev, output_buffer,
				       dev->payload_bytes_per_chunk);
	if (ret != 0) {
		printf("Expecting decryption function to return 0.");
		return -1;
	}

	dprintf("Compare plaintext buffers\n");
	if (memcmp(start_buffer, output_buffer,
		   dev->payload_bytes_per_chunk) != 0) {
		printf("Plaintext buffer compare failed\n");
		return -1;
	}
	return 0;
}

static int do_tests(struct yaffs_tester *tester)
{
	int ret;
	int fail_count = 0;

	dprintf("\nStarting tests\n");
	ret = test_erased_chunk_read(tester);
	if(ret < 0) {
		printf("Encryptor enc_decrypt_fn() should not modify an erased page.\n");
		fail_count++;
	}  else {
		dprintf("test_erased_chunk_read() passed.\n\n");
	}

	ret = test_basic_encrypt_decrypt(tester);
	if(ret < 0) {
		printf("Basic encrypt/decrypt failed\n");
		fail_count++;
	}  else {
		dprintf("test_basic_encrypt_decrypt() passed.\n\n");
	}
	return -fail_count;
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
		printf("Errors in %d tests\n", -ret);
	}

	if (ret)
		return 1;

	return 0;
}

