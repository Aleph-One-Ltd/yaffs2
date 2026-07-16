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
 * yaffscfg2k.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

#include "yaffscfg.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "yaffs_fileem2k.h"
#include "yaffs_nandemul2k.h"
#include "yaffs_trace.h"
#include "yaffs_osglue.h"
#include "yaffs_nandsim_file.h"
#include "yaffs_encrypt_mbedtls.h"

#include <errno.h>

unsigned yaffs_trace_mask =

	YAFFS_TRACE_SCAN |
	YAFFS_TRACE_GC |
	YAFFS_TRACE_ERASE |
	YAFFS_TRACE_ERROR |
	YAFFS_TRACE_TRACING |
	YAFFS_TRACE_ALLOCATE |
	YAFFS_TRACE_BAD_BLOCKS |
	YAFFS_TRACE_VERIFY |
	0;



/* Configure the devices that will be used */

#include "yaffs_flashif2.h"
#include "yaffs_m18_drv.h"
#include "yaffs_nor_drv.h"
#include "yaffs_nand_drv.h"


u8 key[16] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	       0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

int yaffs_start_up(void)
{
	struct yaffs_dev *dev;

	static int start_up_called = 0;

	if(start_up_called)
		return 0;
	start_up_called = 1;

	/* Call the OS initialisation (eg. set up lock semaphore */
	yaffsfs_OSInitialisation();

	/* Install the various devices and their device drivers.
	 * If they do not have encryption, then the default encryption layer
	 * (no encryption) is used.
	 *
	 * Note that the test framework expects the backing file names to be
	 * "emfile-xxx" where "xxx" is the mount point name.
	 */
	yflash2_install_drv("yflash2", "emfile-yflash2");
	yaffs_m18_install_drv("M18-1", "emfile-M18-1");
	yaffs_nor_install_drv("nor", "emfile-nor");
	yaffs_nandsim_install_drv("nand", "emfile-nand", 256, 4, 1);
	/*
	 * Set up nand_enc as an encrypted partition by installing the
	 * encryption layer after creating the dev.
	 */
	dev = yaffs_nor_install_drv("nor_enc", "emfile-nor_enc");
	yaffs_encrypt_mbedtls_install(dev, key, 16);

	dev = yaffs_nandsim_install_drv("nand_enc", "emfile-nand_enc", 256, 4, 1);
	yaffs_encrypt_mbedtls_install(dev, key, 16);

	//yaffs_nandsim_install_drv("nand128MB", "emfile-nand128MB", 1024, 4, 1);
	return 0;
}



