#ifndef __YAFFS_FLASH_SIM_H__
#define __YAFFS_FLASH_SIM_H__

struct yaffs_dev;

struct yaffs_dev *yaffs_rtems_flashsim_setup(void);

void yaffs_rtems_flashsim_dump_status(void);

#endif

