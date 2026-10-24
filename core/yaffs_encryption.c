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

#include "yaffs_encryption.h"

/*
 * Encryption is now supported for all modes, so just return true (1).
 */
int yaffs_has_encryption_layer(struct yaffs_dev *dev)
{
	(void) dev;

	return 1;
}


/*
 * Null encryption functions.
 * None of these functions do anything.
 * The transfer buffer is set up to be the data buffer, meaning
 * that reads/writes transfer directly into the data buffer.
 */
static int yaffs_enc_null_encrypt(struct yaffs_dev *dev,
				  const u8 *data, int data_len)
{
	(void) dev;
	(void) data;
	(void) data_len;

	return 0;
}

static int yaffs_enc_null_decrypt(struct yaffs_dev *dev,
				  u8 *data, int data_len)
{
	(void) dev;
	(void) data;
	(void) data_len;

	return 0;
}

static 	u8 * yaffs_enc_null_get_read_buffer(struct yaffs_dev *dev,
					    u8 *data_buffer)
{
	(void) dev;

	return data_buffer;
}

static 	const u8 * yaffs_enc_null_get_write_buffer(struct yaffs_dev *dev,
						   const u8 *data_buffer)
{
	(void) dev;

	return data_buffer;
}

static int yaffs_enc_null_deinitialise(struct yaffs_dev *dev)
{
	(void) dev;

	return 0;
}

/*
 * Set up a null encryption layer.
 */
static int yaffs_encryption_initialise_null(struct yaffs_dev *dev)
{
	dev->enc.enc_encrypt_fn = yaffs_enc_null_encrypt;
	dev->enc.enc_decrypt_fn = yaffs_enc_null_decrypt;
	dev->enc.enc_get_write_buffer_fn = yaffs_enc_null_get_write_buffer;
	dev->enc.enc_get_read_buffer_fn = yaffs_enc_null_get_read_buffer;
	dev->enc.enc_deinitialise_fn = yaffs_enc_null_deinitialise;
	dev->enc.enc_reserved_bytes = 0;
	dev->enc.enc_name = "null_encryption";
	return 0;
}


/*
 * Wrapper functions.
 */

int yaffs_encryption_encrypt(struct yaffs_dev *dev, const u8 *data, int data_len)
{
	return dev->enc.enc_encrypt_fn(dev, data, data_len);
}

int yaffs_encryption_decrypt(struct yaffs_dev *dev, u8 *data, int data_len)
{
	return dev->enc.enc_decrypt_fn(dev, data, data_len);
}

u8 *yaffs_encryption_get_read_buffer(struct yaffs_dev *dev, u8 *buffer)
{
	return dev->enc.enc_get_read_buffer_fn(dev, buffer);
}

const u8 *yaffs_encryption_get_write_buffer(struct yaffs_dev *dev,
					    const u8 *buffer)
{
	return dev->enc.enc_get_write_buffer_fn(dev, buffer);
}

int yaffs_encryption_get_reserved_bytes(struct yaffs_dev *dev)
{
	return dev->enc.enc_reserved_bytes;
}

const char * yaffs_encryption_get_name(struct yaffs_dev *dev)
{
	return dev->enc.enc_name;
}

/*
 * Initialise the encryption layer.
 * If there isn't one then install the null encryptor.
 */

int yaffs_encryption_init(struct yaffs_dev *dev)
{
	if (!yaffs_has_encryption_layer (dev)) {
		dev->enc.enc_reserved_bytes = 0;
		dev->enc.enc_name = "encryption_disabled";
		return 0;
	}

	if(dev->enc.enc_initialise_fn)
		return dev->enc.enc_initialise_fn(dev);

	return yaffs_encryption_initialise_null(dev);
}

int yaffs_encryption_deinit(struct yaffs_dev *dev)
{
	if (yaffs_has_encryption_layer (dev) &&
	   dev->enc.enc_deinitialise_fn)
		return dev->enc.enc_deinitialise_fn(dev);

	return 0;
}
