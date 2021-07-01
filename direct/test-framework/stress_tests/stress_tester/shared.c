/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * error_handler.c contains code for checking yaffs function calls for errors.
 */
#include "shared.h"
#include "yaffsfs.h"

void quit_program(){
	yaffs_unmount(YAFFS_MOUNT_DIR);
	exit(1);
}
