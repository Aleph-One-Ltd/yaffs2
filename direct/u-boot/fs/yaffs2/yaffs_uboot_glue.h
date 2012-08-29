
#ifndef __YAFFS_UBOOT_GLUE_H__
#define __YAFFS_UBOOT_GLUE_H__


int cmd_yaffs_dev_ls(void);
int cmd_yaffs_tracemask(unsigned set, unsigned mask);
int cmd_yaffs_devconfig(char *mp, int flash_dev,
				int start_block, int end_block);
int cmd_yaffs_mount(char *mp);
int cmd_yaffs_umount(char *mp);
int cmd_yaffs_read_file(char *fn);
int cmd_yaffs_write_file(char *fn, char bval, int sizeOfFile);
int cmd_yaffs_ls(const char *mountpt, int longlist);
int cmd_yaffs_check(const char *fn, const char *type);
int cmd_yaffs_mwrite_file(char *fn, char *addr, int size);
int cmd_yaffs_mread_file(char *fn, char *addr);
int cmd_yaffs_mkdir(const char *dir);
int cmd_yaffs_rmdir(const char *dir);
int cmd_yaffs_rm(const char *path);
int cmd_yaffs_mv(const char *oldPath, const char *newPath);

int yaffs_dump_dev(const char *path);
#endif
