/*
 * YAFFS port to RTEMS
 *
 * Copyright (C) 2010, 2011 Sebastien Bourdeauducq
 * Copyright (C) 2011 Stephan Hoffmann <sho@reLinux.de>
 * Copyright (C) 2011 embedded brains GmbH <rtems@embedded-brains.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * As a special exception, linking other files with the object code from
 * this one to produce an executable application does not by itself cause
 * the resulting executable application to be covered by the GNU General
 * Public License.
 * This exception does not however invalidate any other reasons why the
 * executable file might be covered by the GNU Public License. In particular,
 * the other YAFFS files are not covered by this exception, and using them
 * in a proprietary application requires a paid license from Aleph One.
 */

#include <stdlib.h>
#include <time.h>

#include "yaffs_trace.h"
#include "yaffs_osglue.h"

unsigned int yaffs_trace_mask = YAFFS_TRACE_BAD_BLOCKS | YAFFS_TRACE_ALWAYS;

unsigned int yaffs_wr_attempts;

void *yaffsfs_malloc(size_t size)
{
	return malloc(size);
}

void yaffsfs_free(void *ptr)
{
	free(ptr);
}

YTIME_T yaffsfs_CurrentTime(void)
{
	return time(NULL);
}
