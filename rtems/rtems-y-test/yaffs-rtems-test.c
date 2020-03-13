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

//#define YPATH "/yaffs_mount_pt/"
#define YPATH ""

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
                              NULL /*mount_target */,
                              RTEMS_FILESYSTEM_TYPE_YAFFS,
                              RTEMS_FILESYSTEM_READ_WRITE,
                              &mount_args) < 0) {
		return errno;
	} else {
		return 0;
	}
}


void set_uint8_t_buffer(uint8_t *buf, uint32_t n, uint8_t start, uint8_t inc)
{
	while (n) {
		*buf = start;
		buf++;
		start += inc;
		n--;
	}
}

int run_basic_file_test(void)
{
	int fd;
	int ret;
	uint8_t buf[100];
	uint8_t buf2[100];

	fd = open(YPATH"/test", O_RDWR | O_CREAT | O_TRUNC, 0666);
	printf("open = %d\n", fd);

	set_uint8_t_buffer(buf, sizeof(buf), 0xAA, 1);

	ret = write(fd, buf, sizeof(buf));

	printf("write returned %d\n", ret);

	if (ret == -1)
		perror("writing file");

	ret = lseek(fd, 0, SEEK_END);

	printf("lseek end ret = %d\n", ret);

	ret = lseek(fd, 0, SEEK_SET);
	printf("lseek start ret = %d\n", ret);

	ret = read(fd, buf2, sizeof(buf2));

	printf("reading file ret = %d\n", ret);

	if (ret == -1)
		perror("reading file");


	return ret;

#if 0


   fd = open("test1", O_CREAT);
   printf( "fcntl flags =0x%x\n", fcntl( fd, F_GETFL ) );
   close(fd);

   fd = open("test", O_RDONLY);
   if (fd == -1) {
     printf("Starting on the wrong foot....\n");
     exit(-1);
   }

   printf( "fcntl flags =0x%x\n", fcntl( fd, F_GETFL ) );

   fp = fdopen(fd, "r");
   if (fp == NULL) {
      printf("Nothing ever goes my way!\n");
      close(fd);
      exit(-1);
   } else {
      printf("Soon, I will be able to take over the world!\n");
      fgets(str, 200, fp);
      printf("%s\n", str);
      fclose(fp);
   }
#endif
}


rtems_task Init(
  rtems_task_argument ignored
)
{
	int err;

	 printf("Starting\n");

	err = filesystem_init(YPATH);

	printf("filesystem_init(\"%s\") returned %d\n", YPATH, err);

	run_basic_file_test();


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
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 6

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_MAXIMUM_TASKS 1

#define CONFIGURE_MAXIMUM_SEMAPHORES        20

#define CONFIGURE_INIT

#include <rtems/confdefs.h>
/* end of file */
