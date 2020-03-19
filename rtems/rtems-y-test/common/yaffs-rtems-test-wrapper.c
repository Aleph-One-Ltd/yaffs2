/*
 *  Simple test program -- demonstrating use of IMFS
 */

#include <bsp.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rtems/libio.h>
#include <yaffs/rtems_yaffs.h>

#include "yaffs-rtems-flashsim.h"

#define YPATH "/yaffs_mount_pt"
//#define YPATH ""

void yaffs_bug_fn(const char *file_name, int line_no)
{
	printf("Yaffs bug at %s:%d\n", file_name, line_no);
}

int filesystem_init(const char *mount_target)
{
	struct yaffs_dev *device;
	rtems_yaffs_default_os_context *os_context;
	rtems_yaffs_mount_data mount_args;


	rtems_filesystem_register(RTEMS_FILESYSTEM_TYPE_YAFFS,
				  rtems_yaffs_mount_handler);

	// We mount the filesystem by passing an appropriate
	// rtems_yaffs_mount_data as the last argument to mount(). mount_data is
	// used to pass a yaffs_dev pointer by-value.


	device = yaffs_rtems_flashsim_setup();

	// Initialize callback storage for RTEMS's VFS inside the yaffs_dev.
	os_context = malloc(sizeof(rtems_yaffs_default_os_context));
	rtems_yaffs_initialize_default_os_context(os_context);

	device->os_context = os_context;

	mount_args.dev = device;

	if (mount_and_make_target_path(NULL,
                              mount_target,
                              RTEMS_FILESYSTEM_TYPE_YAFFS,
                              RTEMS_FILESYSTEM_READ_WRITE,
                              &mount_args) < 0) {
		perror("mount_and_make");
		return errno;
	} else {
		chmod(mount_target, 0777); /* Make partition rw/modifiable */
		return 0;
	}
}

extern int run_the_test(void);

rtems_task Init(
  rtems_task_argument ignored
)
{
	int err;

	 printf("Starting\n");

	err = filesystem_init(YPATH);

	printf("filesystem_init(\"%s\") returned %d\n", YPATH, err);

	run_the_test();

	yaffs_rtems_flashsim_dump_status();

   exit(0);
}




#if 0
So basically, we are registering our NAND-specific callbacks with YAFFS
and registering the RTEMS-YAFFS filesystem callbacks with RTEMS.
The rtems_filesystem_register() associates the mount() system call with
a callback function to handle that system call, in this case
rtems_yaffs_mount_handler().  rtems_yaffs_mount_handler() and
RTEMS_FILESYSTEM_TYPE_YAFFS (just a string) are provided
by the rtems-yaffs fork.

mount_and_make_target_path() is provided by RTEMS: it combines a
mkdir -p` with mount(), passing the mount_args to the
previously-registered handler.
#endif


/* configuration information */

/* NOTICE: the clock driver is explicitly disabled */
#define CONFIGURE_APPLICATION_DOES_NOT_NEED_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS 1

#define CONFIGURE_MAXIMUM_SEMAPHORES        20

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
/* end of file */
