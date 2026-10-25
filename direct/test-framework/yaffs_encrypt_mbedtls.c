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
 * onto an existing device BEFORE the device is mounted. This is the INSTALL
 * function.
 *
 * States:
 * * Once a device has been created, but before it is mounted, the encryptor is
 * installed.
 * * During the mounting (yaffs_guts_initialise), the encryptor in initialised
 * by calling the _init_fn.
 * * After that, during use the 4 main functions are used:
 * - get_read_buffer_fn() gets the address of the read buffer that the driver
 *   must read the encrypted flash data into.
 * - get_write_buffer_fn() gets the address of the write buffer that the driver
 *   must write the encrypted flash data from.
 * - encrypt_fn(): The encryption function used to encrypt data into the write
 *   buffer.
 * - decrypt_fn(): The decryption function used to decrypt data from the read
 *   buffer.
 *
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
 * Inband tags:
 * +-------------------------------------------------------+-------------------+
 * |   NAND Data Page area (2048 bytes)                    | Spare (64 bytes)  |
 * |   File system info    (inc tags)                      | Bad block, ECC etc|
 * +-----------+-------------------------------------------+-------------------+
 * | Encryption|   Encryped payload     (2032 bytes)       |
 * | IV (16 b) |  File system info                         |
 * +-----------+----------------+-----------+--------------+
 *             |   File system data (2016 b)| Tags (16 b)  |
 *             |   Header chunk or file data| Metadata     |
 *             +----------------------------+--------------+
 *
 * OOB tags:
 * +-------------------------------------------------+-------------------------+
 * |   NAND Data Page area (2048 bytes)              | Spare (64 bytes)        |
 * |   File system info                              | Tags, Bad block, ECC etc|
 * +-----------+-------------------------------------+-------------------------+
 * | Encryption|  Encryped payload     (2032 bytes)  |
 * | IV (16 b) |   File system info                  |
 * +-----------+-------------------------------------+
 *             |   File system data (2032 bytes)     |
 *             |   Header chunk or file data         |
 *             +-------------------------------------+
 *
 * Thus, for inband tags:
 * - Both data and tags are in the encrypted payload.
 *
 * And, for oob tags:
 * - Data is encrypted, tags are not.
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

/* To run a dummy check without actually encrypting, set JUST_COPY */
//#define JUST_COPY 1

/*
 * Note the name should NOT have any spaces or such.
 * This is for the test tools.
 */
#define ENCRYPTOR_NAME  "yaffs_mbedtls_encryptor"

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

/*
 * The get_write_buffer function returns the address of the buffer that will
 * hold the encrypted data after encryption.
 *
 * Similarly the get_read_buffer function returns the address of the  buffer
 * that will hold the encrypted data before encryption.
 *
 * These addresses are passed to the driver during flash write/read.
 *
 * Here we only have one encrypted buffer (ie only one read or write happening
 * at a time) so the context only uses one buffer for both.
 *
 * NB: We assume this address is known BEFORE the encryption occurs.
 */
static const u8 *yaffs_encrypt_mbedtls_get_write_buffer(struct yaffs_dev *dev,
							const u8 *data_buffer)
{
	struct yaffs_enc_context *context = dev_to_context(dev);

	(void) data_buffer; /* Not used here. */

	return context->encrypted_buffer;
}

static u8 *yaffs_encrypt_mbedtls_get_read_buffer(struct yaffs_dev *dev,
						     u8 *data_buffer)
{
	struct yaffs_enc_context *context = dev_to_context(dev);

	(void) data_buffer;

	return context->encrypted_buffer;
}

/*
 * _encrypt() performs the encryption from the specified data buffer into the
 * context->encrypted_buffer.
 * This function is expected to return 0 for a success.
 */
static int yaffs_encrypt_mbedtls_encrypt(struct yaffs_dev *dev,
					 const u8 *data, int data_len)
{
	struct yaffs_enc_context *context = dev_to_context(dev);
	int eret = 0;
	u8 *encrypt_payload;

	encrypt_payload = &context->encrypted_buffer[IV_SIZE_BYTES];

	/*
	 * Copy the IV inot into the encrypted buffer.
	 * We use the first byte of the stored IV to determine if the chunk
	 * is empty. Force it to a non-FF value.
	 */
	context->iv[0] = 'Y';

	memcpy(context->encrypted_buffer, context->iv, IV_SIZE_BYTES);

#if JUST_COPY
	(void) eret;
	memcpy(encrypt_payload, data, data_len);
#else
	/*
	 * mbedtls AES encryption operations. Set the key on each use.
	 * mbedtls updates the IV on each use.
	 */
	mbedtls_aes_setkey_enc(&context->aes, context->key, 128);
	eret = mbedtls_aes_crypt_cbc(&context->aes, MBEDTLS_AES_ENCRYPT,
			data_len, context->iv,
			data, encrypt_payload);

	/*
	 * encrypted_buffer now holds the 16-byte IV followed by the encrypted
	 * payload.
	 */
	if (eret)
		printf("encryption eret = %d\n", eret);
#endif

	return eret;
}

/*
 * decryption is mostly the mirror of encryption except for special handling of
 * erased chunks. Erased chunks are NOT decrypted, but are passed through as is.
 * We use the first byte of the IV to determine if a chunk is erased or not.
 * Before this function is called, the encrypted_buffer has been filled by the
 * driver reading flash.
 * This function is expected to return 0.
 */
static int yaffs_encrypt_mbedtls_decrypt(struct yaffs_dev *dev,
					 u8 *data, int data_len)
{
	struct yaffs_enc_context *context = dev_to_context(dev);
	int eret = 0;
	u8 *decrypt_iv;
	u8 *decrypt_payload;

	/*
	 * Now we need to check if the chunk was erased. We can do that by
	 * checking the read-back IV is not 0xff.
	 */
	decrypt_iv = context->encrypted_buffer;
	decrypt_payload = &context->encrypted_buffer[IV_SIZE_BYTES];

	if (decrypt_iv[0] == 0xff) {
		/*
		 * The first byte of the IV area is FF, so the chunk is erased.
		 * Write all FF to the output buffer.
		 */

		memset(data, 0xff, data_len);

	} else {
		/*
		 * Not erased, so decrypt into the data buffer.
		 */

#if JUST_COPY
		(void) eret;
		memcpy(data, decrypt_payload, data_len);
#else
		/*
		 * Call the mbedtls AES functions, first setting the key.
		 * mbedtls seems to need the key set.
		 */
		mbedtls_aes_setkey_dec(&context->aes, context->key, 128);
		eret = mbedtls_aes_crypt_cbc(&context->aes, MBEDTLS_AES_DECRYPT,
				    data_len, decrypt_iv,
				    decrypt_payload,
				    data);
		if (eret)
			printf("decryption eret = %d\n", eret);
#endif
	}
	return eret;
}

/*
 *
 * Just use mbedtls to randomise the IV. Could use other functions too.
 * This is only used once at start up. After that the AES undates the IV
 * after each use.
 *
 * NOTE: This is just a place holder that is not particularly secure.
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
 * This is called during mount.
 * This function is expected to return 0.
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
 * Free the context etc.
 * Called during unmout.
 * This function is expected to return 0.
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
 * Note that this should do the bare minimum context initialisation.
 * This function is expected to return 0.
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

	/*
	 * Hook up the context to the device so we can find it later.
	 */
	dev->encryption_context = new_context;

	/*
	 * Set up the encryptor interface.
	 */
	enc = &dev->enc;
	enc->enc_initialise_fn = yaffs_encrypt_mbedtls_init;
	enc->enc_deinitialise_fn = yaffs_encrypt_mbedtls_deinit;
	enc->enc_decrypt_fn = yaffs_encrypt_mbedtls_decrypt;
	enc->enc_encrypt_fn = yaffs_encrypt_mbedtls_encrypt;
	enc->enc_get_read_buffer_fn = yaffs_encrypt_mbedtls_get_read_buffer;
	enc->enc_get_write_buffer_fn = yaffs_encrypt_mbedtls_get_write_buffer;
	enc->enc_reserved_bytes =IV_SIZE_BYTES;
	enc->enc_name = ENCRYPTOR_NAME;
	return 0;
}
