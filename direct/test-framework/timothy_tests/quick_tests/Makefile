# Makefile for quick_tests
#
#
# YAFFS: Yet another Flash File System. A NAND-flash specific file system.
#
# Copyright (C) 2002-2018 Aleph One Ltd.
#
#
# Created by Timothy Manning <timothy@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# NB Warning this Makefile does not include header dependencies.
#

YDI_DIR = ../../../
YDI_FRAMEWORK_DIR = ../../
CLEAN_OBJS = quick_tests emfile-2k-0

TESTFILES = 	quick_tests.o lib.o \
		test_yaffs_mount.o test_yaffs_mount_ENODEV.o test_yaffs_mount_ENAMETOOLONG.o test_yaffs_mount_EBUSY.o \
		test_yaffs_mount_NULL.o \
		test_yaffs_unmount.o test_yaffs_unmount_ENODEV.o test_yaffs_unmount_ENAMETOOLONG.o test_yaffs_unmount_EBUSY.o \
		test_yaffs_open.o test_yaffs_open_ENOENT.o test_yaffs_open_ENOTDIR.o test_yaffs_open_EEXIST.o test_yaffs_open_EISDIR.o \
		test_yaffs_open_ENAMETOOLONG.o test_yaffs_open_EINVAL.o test_yaffs_open_EINVAL2.o test_yaffs_open_ELOOP.o \
		test_yaffs_open_ELOOP_dir.o test_yaffs_open_EROFS.o test_yaffs_open_EACCES.o test_yaffs_open_NULL.o \
		test_yaffs_unlink.o  test_yaffs_unlink_EISDIR.o test_yaffs_unlink_ENOENT.o test_yaffs_unlink_ENAMETOOLONG.o \
		test_yaffs_unlink_ENOTDIR.o test_yaffs_unlink_ENOENT2.o test_yaffs_unlink_ELOOP_dir.o test_yaffs_unlink_EROFS.o \
		test_yaffs_unlink_NULL.o \
		test_yaffs_ftruncate.o test_yaffs_ftruncate_EBADF.o test_yaffs_ftruncate_EINVAL.o test_yaffs_ftruncate_big_file.o \
		test_yaffs_ftruncate_EROFS.o \
		test_yaffs_truncate.o test_yaffs_truncate_ENOTDIR.o test_yaffs_truncate_EISDIR.o test_yaffs_truncate_ENOENT.o \
		test_yaffs_truncate_EINVAL.o test_yaffs_truncate_big_file.o test_yaffs_truncate_ENOENT2.o test_yaffs_truncate_ELOOP.o \
		test_yaffs_truncate_ELOOP_dir.o test_yaffs_truncate_EROFS.o test_yaffs_truncate_ENAMETOOLONG.o \
		test_yaffs_truncate_NULL.o \
		test_yaffs_write.o test_yaffs_write_EBADF.o test_yaffs_write_big_file.o test_yaffs_write_EROFS.o \
		test_yaffs_read.o test_yaffs_read_EBADF.o test_yaffs_read_EINVAL.o\
		test_yaffs_lseek.o test_yaffs_lseek_EBADF.o test_yaffs_lseek_EINVAL.o test_yaffs_lseek_big_file.o \
		test_yaffs_access.o test_yaffs_access_EINVAL.o test_yaffs_access_ENOTDIR.o test_yaffs_access_ENOENT.o \
		test_yaffs_access_ENOENT2.o test_yaffs_access_ELOOP_dir.o test_yaffs_access_ELOOP.o test_yaffs_access_EROFS.o \
		test_yaffs_access_EACCES.o test_yaffs_access_ENAMETOOLONG.o test_yaffs_access_NULL.o \
		test_yaffs_stat.o test_yaffs_stat_ENOENT.o test_yaffs_stat_ENOTDIR.o test_yaffs_stat_ENOENT2.o test_yaffs_stat_ELOOP.o \
		test_yaffs_stat_ELOOP_dir.o test_yaffs_stat_ENAMETOOLONG.o test_yaffs_stat_NULL.o \
		test_yaffs_fstat.o test_yaffs_fstat_EBADF.o  \
		test_yaffs_close_EBADF.o test_yaffs_close_NULL.o \
		test_yaffs_chmod.o test_yaffs_chmod_ENOENT.o test_yaffs_chmod_ENOTDIR.o test_yaffs_chmod_EINVAL.o \
		test_yaffs_chmod_ENOENT2.o test_yaffs_chmod_ELOOP_dir.o test_yaffs_chmod_ELOOP.o test_yaffs_chmod_EROFS.o \
		test_yaffs_chmod_ENAMETOOLONG.o test_yaffs_chmod_NULL.o \
		test_yaffs_fchmod_EROFS.o test_yaffs_fchmod_EINVAL.o\
		test_yaffs_fsync.o test_yaffs_fsync_EBADF.o test_yaffs_fsync_EROFS.o \
		test_yaffs_fdatasync.o test_yaffs_fdatasync_EBADF.o test_yaffs_fdatasync_EROFS.o test_yaffs_fdatasync_NULL.o \
		test_yaffs_mkdir.o test_yaffs_mkdir_EEXIST.o test_yaffs_mkdir_ENOTDIR.o test_yaffs_mkdir_ENOENT.o \
		test_yaffs_mkdir_ELOOP_dir.o test_yaffs_mkdir_EROFS.o test_yaffs_mkdir_ENAMETOOLONG.o test_yaffs_mkdir_NULL.o \
		test_yaffs_fchmod.o test_yaffs_fchmod_EBADF.o  test_yaffs_fchmod_NULL.o \
		test_yaffs_symlink.o test_yaffs_symlink_ENOTDIR.o test_yaffs_symlink_EEXIST.o test_yaffs_symlink_ENOENT.o \
		test_yaffs_symlink_ENOENT2.o test_yaffs_symlink_ELOOP_dir.o test_yaffs_symlink_EROFS.o test_yaffs_symlink_ENAMETOOLONG.o\
		test_yaffs_symlink_NULL.o test_yaffs_symlink_NULL2.o \
		test_yaffs_mount2.o test_yaffs_mount2_ENODEV.o \
		test_yaffs_unmount2.o test_yaffs_unmount2_ENODEV.o test_yaffs_unmount2_EINVAL.o \
		test_yaffs_unmount2_with_handle_open_and_forced_mode_on.o test_yaffs_unmount2_with_handle_open_and_forced_mode_off.o\
		test_yaffs_sync.o test_yaffs_sync_ENODEV.o  test_yaffs_sync_EROFS.o test_yaffs_sync_ENAMETOOLONG.o \
		test_yaffs_sync_NULL.o\
		test_yaffs_remount_force_off_read_only_off.o test_yaffs_remount_force_on_read_only_off.o test_yaffs_remount_ENODEV.o \
		test_yaffs_remount_EINVAL.o test_yaffs_remount_ENAMETOOLONG.o test_yaffs_remount_NULL.o test_yaffs_unmount_NULL.o \
		test_yaffs_freespace.o test_yaffs_freespace_EINVAL.o test_yaffs_freespace_ENAMETOOLONG.o test_yaffs_freespace_NULL.o \
		test_yaffs_totalspace.o test_yaffs_totalspace_EINVAL.o test_yaffs_totalspace_ENAMETOOLONG.o test_yaffs_totalspace_NULL.o\
		test_yaffs_inodecount.o test_yaffs_inodecount_EINVAL.o test_yaffs_inodecount_ENAMETOOLONG.o test_yaffs_inodecount_NULL.o\
		test_yaffs_link.o test_yaffs_link_ENOENT.o test_yaffs_link_EEXIST.o test_yaffs_link_ENOTDIR.o \
		test_yaffs_link_ENOTDIR2.o test_yaffs_link_ENOENT2.o test_yaffs_link_ENOENT3.o  \
		test_yaffs_link_ENOENT4.o test_yaffs_link_ELOOP_dir.o test_yaffs_link_EROFS.o test_yaffs_link_ENAMETOOLONG.o \
		test_yaffs_link_ENAMETOOLONG2.o test_yaffs_link_NULL.o test_yaffs_link_NULL2.o \
		test_yaffs_rmdir.o test_yaffs_rmdir_EBUSY.o test_yaffs_rmdir_EINVAL.o test_yaffs_rmdir_ENOENT.o \
		test_yaffs_rmdir_ENOTDIR.o test_yaffs_rmdir_ELOOP_dir.o test_yaffs_rmdir_EROFS.o test_yaffs_rmdir_ENAMETOOLONG.o \
		test_yaffs_rmdir_ENOTEMPTY.o \
		test_yaffs_rename.o test_yaffs_rename_ENOENT.o test_yaffs_rename_ENOTDIR.o test_yaffs_rename_EINVAL.o \
		test_yaffs_rename_dir.o test_yaffs_rename_dir_ENOENT.o test_yaffs_rename_dir_ENOENT2.o test_yaffs_rename_dir_to_file.o \
		test_yaffs_rename_file_to_dir.o test_yaffs_rename_EEXISTS.o  test_yaffs_rename_ELOOP_dir.o test_yaffs_rename_EROFS.o \
		test_yaffs_rename_ENAMETOOLONG.o test_yaffs_rename_ENAMETOOLONG2.o test_yaffs_rename_NULL.o test_yaffs_rename_NULL2.o \
		test_yaffs_lstat.o test_yaffs_lstat_ENOENT.o test_yaffs_lstat_ENOTDIR.o test_yaffs_lstat_ENAMETOOLONG.o test_yaffs_rename_to_null_file.o \
		test_yaffs_rename_file_over_file.o test_yaffs_rename_dir_over_dir.o test_yaffs_rename_full_dir_over_dir.o test_yaffs_rename_ENOTEMPTY.o \
		test_yaffs_lstat_NULL.o \
		test_yaffs_flush.o test_yaffs_flush_EBADF.o test_yaffs_flush_EROFS.o \
		test_yaffs_dup.o test_yaffs_dup_EBADF.o 


all: quick_tests

YAFFS_TEST_OBJS = $(COMMONTESTOBJS) $(TESTFILES)
FUZZER_OBJS = fuzzer.o

ALL_UNSORTED_OBJS += $(YAFFS_TEST_OBJS) $(FUZZER_OBJS)

include $(YDI_FRAMEWORK_DIR)/FrameworkRules.mk


yaffs_test: $(FRAMEWORK_SOURCES) $(YAFFS_TEST_OBJS)
	gcc $(CFLAGS) -o $@ $(YAFFS_TEST_OBJS) -lpthread



quick_tests: $(FRAMEWORK_SOURCES) $(YAFFS_TEST_OBJS)
	gcc $(CFLAGS) -o $@ $(YAFFS_TEST_OBJS) -lpthread

