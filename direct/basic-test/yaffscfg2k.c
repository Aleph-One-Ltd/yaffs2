/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
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
#include "yaffs_norif1.h"
#include "yaffs_trace.h"
#include "yaffs_osglue.h"


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



// Configuration

#include "yaffs_ramdisk.h"
#include "yaffs_flashif.h"
#include "yaffs_flashif2.h"
#include "yaffs_nandemul2k.h"

struct yaffs_dev ram1Dev;
struct yaffs_dev flashDev;
struct yaffs_dev m18_1Dev;

int yaffs_start_up(void)
{
	static int start_up_called = 0;

	if(start_up_called)
		return 0;
	start_up_called = 1;

	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_OSInitialisation();

	// Set up devices

	// /M18-1 yaffs1 on M18 nor sim
	memset(&m18_1Dev,0,sizeof(m18_1Dev));
	ynorif1_install_drv(&m18_1Dev);
	m18_1Dev.param.name = "M18-1";
	m18_1Dev.param.n_caches = 10; // Use caches
	m18_1Dev.param.disable_soft_del = 1;
	m18_1Dev.driver_context = (void *) 1;	// Used to identify the device in fstat.


//	m18_1Dev.param.disable_soft_del = 1;

	yaffs_add_device(&m18_1Dev);

	// /yaffs2  yaffs2 file emulation
	// 2kpage/64chunk per block
	//
	memset(&flashDev,0,sizeof(flashDev));
	yflash2_install_drv(&flashDev);
	flashDev.param.name = "yaffs2";

	flashDev.param.n_reserved_blocks = 5;
	flashDev.param.wide_tnodes_disabled=0;
	flashDev.param.refresh_period = 1000;
	flashDev.param.n_caches = 10; // Use caches
	flashDev.driver_context = (void *) 2;	// Used to identify the device in fstat.

	flashDev.param.enable_xattr = 1;

	yaffs_add_device(&flashDev);

	return 0;
}



