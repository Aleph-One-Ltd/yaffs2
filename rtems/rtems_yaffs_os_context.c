/*
 * YAFFS port to RTEMS
 *
 * Copyright (c) 2011 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
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

#include "rtems_yaffs.h"

#include <assert.h>
#include <errno.h>

static void rtems_yaffs_default_lock(struct yaffs_dev *dev, void *arg)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	rtems_yaffs_default_os_context *os_context = arg;

        sc = rtems_semaphore_obtain(
		os_context->semaphore_id,
		RTEMS_WAIT,
		RTEMS_NO_TIMEOUT
	);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void rtems_yaffs_default_unlock(struct yaffs_dev *dev, void *arg)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	rtems_yaffs_default_os_context *os_context = arg;

        sc = rtems_semaphore_release(os_context->semaphore_id);
	assert(sc == RTEMS_SUCCESSFUL);
}

static void rtems_yaffs_default_unmount(struct yaffs_dev *dev, void *arg)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;
	rtems_yaffs_default_os_context *os_context = arg;

        sc = rtems_semaphore_delete(os_context->semaphore_id);
	assert(sc == RTEMS_SUCCESSFUL);
}

int rtems_yaffs_initialize_default_os_context(
	rtems_yaffs_default_os_context *os_context
)
{
	rtems_status_code sc = RTEMS_SUCCESSFUL;

	os_context->os_context.lock = rtems_yaffs_default_lock;
	os_context->os_context.unlock = rtems_yaffs_default_unlock;
	os_context->os_context.unmount = rtems_yaffs_default_unmount;

	sc = rtems_semaphore_create(
		rtems_build_name('Y', 'A', 'F', 'S'),
		1,
		RTEMS_LOCAL
			| RTEMS_BINARY_SEMAPHORE
			| RTEMS_INHERIT_PRIORITY
			| RTEMS_PRIORITY,
		0,
		&os_context->semaphore_id
	);
	if (sc == RTEMS_SUCCESSFUL) {
		return 0;
	} else {
		errno = ENOMEM;

		return -1;
	}
}
