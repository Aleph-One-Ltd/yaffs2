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
 * The installer function just installs the various field: handler functions and
 * some values the rest of Yaffs needs to know about and a name (just for
 * debugging purposes).
 */
int yaffs_encrypt_mbedtls_install(struct yaffs_dev *dev)
{
	struct yaffs_encryptor *enc = &dev->enc;

	enc->enc_initialise_fn = NULL;
	enc->enc_deinitialise_fn = NULL;
	enc->enc_read_chunk_fn = NULL;
	enc->enc_write_chunk_fn = NULL;
	enc->enc_reserved_bytes = 0;
	enc->enc_name = "yaffs mbedtls encryptor";
	return 0;
}
