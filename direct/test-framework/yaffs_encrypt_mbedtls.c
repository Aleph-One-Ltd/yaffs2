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
 */

/*
 * This file provides an encryption layer based on mbedTLS using AES.
 * This also serves as a boilerplate example of how to construct an
 * encryptor for Yaffs.
 *
 * This layer only exposes one function which is used to install the encryptor
 * onto an existing device BEFORE the device is mounted.
 */

#include "yaffs_encrypt_mbedtls.h"
#include "yaffs_guts.h"

/*
 * How this layer works.
 * This layer uses MbedTLS encryption library for its AES encryption functions.
 * Pretty much any other AES implementation should be easy to substitute in.
 * Here we are using 128 bit blocks (16 bytes) the AES standard.
 * There is an initial vector (IV) of 128 bits (16 bytes). The IV is randomised
 * at start up and is automatically updated by each encryption call, therefore
 * only needs a single initialisation.
 *
 * The chunk (typically a nand page) typically has the following parts as far
 * as the driver is concerned:
 * the data part of the nand page and the spare area used for ECC, bad block
 * markers and such.
 *
 * The encryption layer just operates in the NAND Data Page Area and splits
 * up the data page area into encrypted fie sytem data and the reserved bytes
 * used by the encryption layer. Here we use 16 bytes to store the IV.
 *
 * The encrypted payload is then further split into the data portion (a file
 * header chuck or file data) and tags, but that is invisible to the encryptor.
 *
 * +-------------------------------------------------------+-------------------+
 * |   NAND Data Page area (2048 bytes)                    | Spare (64 bytes)  |
 * |   File system info                                    | Bad block, ECC etc|
 * +-------------------------------------------+-----------+-------------------+
 * |   Encryped payload     (2032 bytes)       | Encryption|
 * |   File system info                        | IV (16 b) |
 * +----------------------------+--------------+-----------+
 * |   File system data (2016 b)| Tags (16 b)  |
 * |   Header chunk or file data| Metadata     |
 * +-------------------------------------------+
 */

 /*
  * This is the local context information - private to the functions
  * in this file and stores the implementation-specific context information.
  *
  * mbedTLS AES is used for the encryption and decryption.
  * mbedTLS is used ONLY to initialise the IV, could use other mechanisms too.
  */
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/aes.h"

#define IV_SIZE_BYTES	(16)
#define KEY_SIZE_BYTES	(16)
#define KEY_SIZE_BITS	(KEY_SIZE_BYTES * 8)
#define ENCRYPTOR_NAME  "yaffs mbedtls encryptor"

struct yaffs_enc_context {
	u8 iv[IV_SIZE_BYTES];
	u8 key[KEY_SIZE_BYTES];
	mbedtls_aes_context aes;
	u8 *encrypted_buffer;
	int initialised;
};

/*
 * Macro to re-cast the void pointer back to the the local context type.
 */
#define dev_to_context(dev) \
	(struct yaffs_enc_context *)((dev)->encryption_context)

static int yaffs_encrypt_mbedtls_write_chunk(
				   struct yaffs_dev *dev, int nand_chunk,
				   const u8 *data, int data_len,
				   const u8 *oob, int oob_len)
{
	struct yaffs_enc_context *context = dev_to_context(dev);
	int eret;

	/*
	 * We use the first byte of the stored IV to determine if the chunk is empty.
	 * Force it to a non-FF value.
	 */
	context->iv[0] &= 0x55;

	memcpy(&context->encrypted_buffer[data_len], context->iv, IV_SIZE_BYTES);

#if JUST_COPY
	(void) eret;
	memcpy(context->encrypted_buffer, data, data_len);
#else
	mbedtls_aes_setkey_enc(&context->aes, context->key, 128);
	eret = mbedtls_aes_crypt_cbc(&context->aes, MBEDTLS_AES_ENCRYPT,
				    data_len, context->iv,
				    data, context->encrypted_buffer);
	if (eret)
		printf("encryption eret = %d\n", eret);
#endif

	return dev->drv.drv_write_chunk_fn(dev, nand_chunk,
		context->encrypted_buffer, dev->param.total_bytes_per_chunk,
		oob, oob_len);
}

static int yaffs_encrypt_mbedtls_read_chunk(
				   struct yaffs_dev *dev, int nand_chunk,
				   u8 *data, int data_len,
				   u8 *oob, int oob_len,
				   enum yaffs_ecc_result *ecc_result)
{
	struct yaffs_enc_context *context = dev_to_context(dev);
	int ret;
	int eret;
	u8 *decrypt_iv;

	ret =  dev->drv.drv_read_chunk_fn(dev, nand_chunk,
		context->encrypted_buffer, dev->param.total_bytes_per_chunk,
		oob, oob_len,
		ecc_result);
	/*
	 * Now we need to check if the chunk was empty. We can do that by
	 * checking the read-back IV is not 0xff.
	 */
	decrypt_iv = &context->encrypted_buffer[data_len];

	if (decrypt_iv[0] == 0xff) {
		/*
		 * The first byte of the IV area is FF, so the page is unwritten.
		 * Write FF to the output buffer.
		 */

		memset(data, 0xff, data_len);

	} else {
		/*
		 * Decrypt into the data buffer.
		 */

#if JUST_COPY
		(void) eret;
		memcpy(data, context->encrypted_buffer, data_len);
#else
		mbedtls_aes_setkey_dec(&context->aes, context->key, 128);
		eret = mbedtls_aes_crypt_cbc(&context->aes, MBEDTLS_AES_DECRYPT,
				    data_len, decrypt_iv,
				    context->encrypted_buffer, data);
	if (eret)
		printf("decryption eret = %d\n", eret);
#endif
	}
	return ret;
}

/*
 *
 * Just use mbedtls to randomise the IV. Could use other functions too.
 */
static void randomise_iv(u8 *iv, int iv_size)
{
	int ret;
	/*
	 * The entropy and ctr_drbg are just used for providing random
	 * data for the initial IV data.
	 */

	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;

	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
				(const unsigned char *) "RANDOM JUNK", 12);
	ret = mbedtls_ctr_drbg_random(&ctr_drbg, iv, iv_size);
	if (ret != 0)
		printf("random function returned %d\n", ret);

        mbedtls_entropy_free(&entropy);
        mbedtls_ctr_drbg_free(&ctr_drbg);
}

/*
 * Initialise the context and randomise the IV.
 */
static int yaffs_encrypt_mbedtls_init(struct yaffs_dev *dev)
{
	struct yaffs_enc_context *context = dev_to_context(dev);
	u8 *buffer;

	buffer = kmalloc(dev->param.total_bytes_per_chunk, 0);

	if (!buffer)
		return 0;

	context->encrypted_buffer = buffer;
	mbedtls_aes_init(&context->aes);
	randomise_iv(context->iv, IV_SIZE_BYTES);
	context->initialised = 1;

	return 0;
}

/*
 *  Free the context.
 */
static int yaffs_encrypt_mbedtls_deinit(struct yaffs_dev *dev)
{
	struct yaffs_enc_context *context = dev_to_context(dev);

	if (!context->initialised)
		return -1;

	mbedtls_aes_free(&context->aes);
	kfree(context->encrypted_buffer);
	context->encrypted_buffer = NULL;
	context->initialised = 0;

	return 0;
}

/*
 * The installer function just installs the various field: handler functions and
 * some values the rest of Yaffs needs to know about and a name (just for
 * debugging purposes).
 */
int yaffs_encrypt_mbedtls_install(struct yaffs_dev *dev, const u8 *key,
				  int key_size_bytes)
{
	struct yaffs_enc_context *new_context;
	struct yaffs_encryptor *enc;

	/*
	 * Set up the encryption context.
	 * At this stage we will set up an empty context and just load the key.
	 * The rest of the context will be set up during the initialisation.
	 */

	new_context = malloc(sizeof(struct yaffs_enc_context));
	memset(new_context, 0, sizeof(struct yaffs_enc_context));

	if (key_size_bytes > KEY_SIZE_BYTES)
		key_size_bytes = KEY_SIZE_BYTES;

	memcpy(new_context->key, key, key_size_bytes);

	dev->encryption_context = new_context;

	/*
	 * Set up the encryptor interface.
	 */
	enc = &dev->enc;
	enc->enc_initialise_fn = yaffs_encrypt_mbedtls_init;
	enc->enc_deinitialise_fn = yaffs_encrypt_mbedtls_deinit;
	enc->enc_read_chunk_fn = yaffs_encrypt_mbedtls_read_chunk;
	enc->enc_write_chunk_fn = yaffs_encrypt_mbedtls_write_chunk;
	enc->enc_reserved_bytes =IV_SIZE_BYTES;
	enc->enc_name = ENCRYPTOR_NAME;
	return 0;
}
