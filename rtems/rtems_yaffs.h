/*
 * YAFFS port to RTEMS
 *
 * Copyright (C) 2010 Sebastien Bourdeauducq
 * Copyright (C) 2011 Stephan Hoffmann <sho@reLinux.de>
 * Copyright (C) 2011 embedded brains GmbH <rtems@embedded-brains.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 * As a special exception, including this header in a file does not by
 * itself cause the resulting executable application to be covered by the
 * GNU General Public License.
 * This exception does not however invalidate any other reasons why the
 * executable file might be covered by the GNU Public License. In particular,
 * the other YAFFS files are not covered by this exception, and using them
 * in a proprietary application requires a paid license from Aleph One.
 */

#ifndef __RTEMS_YAFFS_H
#define __RTEMS_YAFFS_H

#include <rtems.h>
#include <rtems/fs.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Must be inside the extern "C" */
#include "yportenv.h"
#include "yaffs_guts.h"

/**
 * @defgroup rtems_yaffs YAFFS Support for RTEMS
 *
 *
 * @{
 */

#define RTEMS_FILESYSTEM_TYPE_YAFFS "yaffs"

typedef void (*rtems_yaffs_os_handler)(
  struct yaffs_dev *dev,
  void *os_context
);

/**
 * @brief Per YAFFS file system instance context.
 */
typedef struct {
  rtems_yaffs_os_handler lock;
  rtems_yaffs_os_handler unlock;
  rtems_yaffs_os_handler unmount;

  /**
   * @brief The device containing the file system instance.
   *
   * This will be used for the st_dev field in stat().
   */
  dev_t dev;
} rtems_yaffs_os_context;

/**
 * @brief Default per YAFFS file system instance context.
 */
typedef struct {
  rtems_yaffs_os_context os_context;
  rtems_id semaphore_id;
} rtems_yaffs_default_os_context;

/**
 * @brief Data for YAFFS mount handler.
 *
 * @see rtems_yaffs_mount_handler()
 */
typedef struct {
  /**
   * @brief YAFFS device of the file system instance.
   *
   * The @a param field has to be completely set up.  The
   * @a driver_context can point to arbitrary driver specific
   * information.  The @a os_context must point to an initialized
   * structure that begins with a rtems_yaffs_os_context structure.
   */
  struct yaffs_dev *dev;
} rtems_yaffs_mount_data;

/**
 * @brief YAFFS mount handler.
 *
 * The @a data pointer must point to a completely initialized
 * rtems_yaffs_mount_data structure.  The ownership of the YAFFS device
 * structure changes.  This structure is now owned by the file system layer.
 *
 * @retval 0 Successful operation.
 * @retval -1 An error occurred.  The @c errno indicates the error.
 */
int rtems_yaffs_mount_handler(
  rtems_filesystem_mount_table_entry_t *mt_entry,
  const void *data
);

/**
 * @brief Initializes the default per file system context @a os_context.
 *
 * A binary semaphore with priority inheritance will be used to ensure mutual
 * exclusion.
 *
 * The umount handler will release all resources of the default context.
 *
 * @retval 0 Successful operation.
 * @retval -1 An error occurred.  The @c errno indicates the error.
 */
int rtems_yaffs_initialize_default_os_context(
  rtems_yaffs_default_os_context *os_context
);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RTEMS_YAFFS_H */
