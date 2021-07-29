/*
 * YAFFS port to RTEMS
 *
 * Copyright (C) 2010, 2011 Sebastien Bourdeauducq
 * Copyright (C) 2011 Stephan Hoffmann <sho@reLinux.de>
 * Copyright (C) 2011-2012 embedded brains GmbH <rtems@embedded-brains.de>
 * Copyright (C) 2019 Space Sciences and Engineering, LLC
 *     <jbrandmeyer@planetiq.com>
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

#include <rtems.h>
#include <rtems/libio_.h>
#include <rtems/seterr.h>
#include <rtems/userenv.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>

#include "yportenv.h"

#include "yaffs_guts.h"
#include "yaffs_trace.h"
#include "yaffs_packedtags2.h"

#include "rtems_yaffs.h"

/* RTEMS interface */

static const rtems_filesystem_file_handlers_r yaffs_directory_handlers;
static const rtems_filesystem_file_handlers_r yaffs_file_handlers;
static const rtems_filesystem_file_handlers_r yaffs_link_handlers;
static const rtems_filesystem_operations_table yaffs_ops;

/* locking */

static void ylock(struct yaffs_dev *dev)
{
	rtems_yaffs_os_context *os_context = dev->os_context;
	(*os_context->lock)(dev, os_context);
}

static void yunlock(struct yaffs_dev *dev)
{
	rtems_yaffs_os_context *os_context = dev->os_context;
	(*os_context->unlock)(dev, os_context);
}

static void rtems_yaffs_os_unmount(struct yaffs_dev *dev)
{
	rtems_yaffs_os_context *os_context = dev->os_context;
	(*os_context->unmount)(dev, os_context);
}

static struct yaffs_obj *ryfs_get_object_by_location(
	const rtems_filesystem_location_info_t *loc
)
{
	return loc->node_access;
}

static struct yaffs_obj *ryfs_get_object_by_iop(
	const rtems_libio_t *iop
)
{
	return iop->pathinfo.node_access;
}

static struct yaffs_dev *ryfs_get_device_by_mt_entry(
	const rtems_filesystem_mount_table_entry_t *mt_entry
)
{
	return mt_entry->fs_info;
}

static void ryfs_set_location(rtems_filesystem_location_info_t *loc, struct yaffs_obj *obj)
{
	loc->node_access = obj;

	switch (obj->variant_type) {
		case YAFFS_OBJECT_TYPE_FILE:
			loc->handlers = &yaffs_file_handlers;
			break;
		case YAFFS_OBJECT_TYPE_DIRECTORY:
			loc->handlers = &yaffs_directory_handlers;
			break;
		case YAFFS_OBJECT_TYPE_SYMLINK:
			loc->handlers = &yaffs_link_handlers;
			break;
		default:
			loc->handlers = &rtems_filesystem_handlers_default;
			break;
	};
}

static bool ryfs_eval_is_directory(
	rtems_filesystem_eval_path_context_t *ctx,
	void *arg
)
{
	rtems_filesystem_location_info_t *currentloc =
		rtems_filesystem_eval_path_get_currentloc(ctx);
	struct yaffs_obj *obj = ryfs_get_object_by_location(currentloc);

	obj = yaffs_get_equivalent_obj(obj);

	return obj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY;
}

static const char *ryfs_make_string(char *buf, const char *src, size_t len)
{
	buf [len] = '\0';

	return memcpy(buf, src, len);
}

static struct yaffs_obj *ryfs_search_in_directory(
	struct yaffs_obj *dir,
	const char *token,
	size_t tokenlen
)
{
	if (rtems_filesystem_is_parent_directory(token, tokenlen)) {
		dir = dir->parent;
	} else if (!rtems_filesystem_is_current_directory(token, tokenlen)) {
		if (tokenlen < YAFFS_MAX_NAME_LENGTH) {
			char buf [YAFFS_MAX_NAME_LENGTH + 1];

			dir = yaffs_find_by_name(
				dir,
				ryfs_make_string(buf, token, tokenlen)
			);
		} else {
			dir = NULL;
		}
	}

	return dir;
}

static rtems_filesystem_eval_path_generic_status ryfs_eval_token(
	rtems_filesystem_eval_path_context_t *ctx,
	void *arg,
	const char *token,
	size_t tokenlen
)
{
	rtems_filesystem_eval_path_generic_status status =
		RTEMS_FILESYSTEM_EVAL_PATH_GENERIC_DONE;
	rtems_filesystem_location_info_t *currentloc =
		rtems_filesystem_eval_path_get_currentloc(ctx);
	struct yaffs_obj *dir = ryfs_get_object_by_location(currentloc);
	bool access_ok = rtems_filesystem_eval_path_check_access(
		ctx,
		RTEMS_FS_PERMS_EXEC,
		dir->yst_mode,
		(uid_t) dir->yst_uid,
		(gid_t) dir->yst_gid
	);

	if (access_ok) {
		struct yaffs_obj *entry = ryfs_search_in_directory(dir, token, tokenlen);

		if (entry != NULL) {
			bool terminal = !rtems_filesystem_eval_path_has_path(ctx);
			int eval_flags = rtems_filesystem_eval_path_get_flags(ctx);
			bool follow_hard_link = (eval_flags & RTEMS_FS_FOLLOW_HARD_LINK) != 0;
			bool follow_sym_link = (eval_flags & RTEMS_FS_FOLLOW_SYM_LINK) != 0;
			enum yaffs_obj_type type = entry->variant_type;

			rtems_filesystem_eval_path_clear_token(ctx);

			if (type == YAFFS_OBJECT_TYPE_HARDLINK && (follow_hard_link || !terminal)) {
				entry = yaffs_get_equivalent_obj(entry);
			}

			if (type == YAFFS_OBJECT_TYPE_SYMLINK && (follow_sym_link || !terminal)) {
				const char *target = entry->variant.symlink_variant.alias;

				rtems_filesystem_eval_path_recursive(ctx, target, strlen(target));
			} else {
				ryfs_set_location(currentloc, entry);

				if (!terminal) {
					status = RTEMS_FILESYSTEM_EVAL_PATH_GENERIC_CONTINUE;
				}
			}
		} else {
			status = RTEMS_FILESYSTEM_EVAL_PATH_GENERIC_NO_ENTRY;
		}
	}

	return status;
}

static const rtems_filesystem_eval_path_generic_config ryfs_eval_config = {
	.is_directory = ryfs_eval_is_directory,
	.eval_token = ryfs_eval_token
};

static void ryfs_eval_path(rtems_filesystem_eval_path_context_t *ctx)
{
	rtems_filesystem_eval_path_generic(ctx, NULL, &ryfs_eval_config);
}

/* Helper functions */

static int ryfs_mknod(
	const rtems_filesystem_location_info_t *parentloc,
	const char *name,
	size_t namelen,
	mode_t mode,
	dev_t dev
)
{
	int rv = 0;
	struct yaffs_obj *parent = ryfs_get_object_by_location(parentloc);
	struct yaffs_obj *(*create)(
		struct yaffs_obj *parent,
		const YCHAR *name,
		u32 mode,
		u32 uid,
		u32 gid
	);

	switch (mode & S_IFMT) {
		case S_IFREG:
			create = yaffs_create_file;
			break;
		case S_IFDIR:
			create = yaffs_create_dir;
			break;
		default:
			errno = EINVAL;
			rv = -1;
			break;
	}

	if (rv == 0) {
		char buf [YAFFS_MAX_NAME_LENGTH + 1];
		struct yaffs_obj *entry = (*create)(
			parent,
			ryfs_make_string(buf, name, namelen),
			mode,
			geteuid(),
			getegid()
		);

		if (entry == NULL) {
			errno = ENOSPC;
			rv = -1;
		}
	}

	return rv;
}

static int ryfs_utime(
	const rtems_filesystem_location_info_t *loc,
	time_t actime,
	time_t modtime
)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_location(loc);

	obj = yaffs_get_equivalent_obj(obj);
	if (obj != NULL) {
		obj->dirty = 1;
		obj->yst_atime = actime;
		obj->yst_mtime = modtime;
		obj->yst_ctime = time(NULL);
	} else {
		errno = EIO;
		rv = -1;
	}

	return rv;
}

static int ryfs_rename(
	const rtems_filesystem_location_info_t *old_parent_loc,
	const rtems_filesystem_location_info_t *old_loc,
	const rtems_filesystem_location_info_t *new_parent_loc,
	const char *name,
	size_t namelen
)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_location(old_loc);
	char old_name_buf [YAFFS_MAX_NAME_LENGTH + 1];
	char new_name_buf [YAFFS_MAX_NAME_LENGTH + 1];
	int yc;

	yaffs_get_obj_name(obj, old_name_buf, sizeof(old_name_buf));
	yc = yaffs_rename_obj(
		obj->parent,
		old_name_buf,
		ryfs_get_object_by_location(new_parent_loc),
		ryfs_make_string(new_name_buf, name, namelen)
	);
	if (yc != YAFFS_OK) {
		errno = EIO;
		rv = -1;
	}

	return rv;
}

static ssize_t ryfs_dir_read(rtems_libio_t *iop, void *buffer, size_t count)
{
	struct yaffs_obj *obj;
	struct yaffs_dev *dev;
	struct dirent *de = (struct dirent *)buffer;
	size_t i;
	size_t maxcount;
	struct list_head *next;
	ssize_t readlen;

	obj = (struct yaffs_obj *)iop->pathinfo.node_access;
	dev = obj->my_dev;
	maxcount = count / sizeof(struct dirent);

	ylock(dev);

	if(iop->offset == 0) {
		if(list_empty(&obj->variant.dir_variant.children))
			iop->data1 = NULL;
		else
			iop->data1 = list_entry(obj->variant.dir_variant.children.next, struct yaffs_obj, siblings);
	}

	i = 0;
	while((i < maxcount) && (iop->data1 != NULL)) {
		de[i].d_ino = (long)yaffs_get_equivalent_obj((struct yaffs_obj *)iop->data1)->obj_id;
		de[i].d_off = 0;
		yaffs_get_obj_name((struct yaffs_obj *)iop->data1, de[i].d_name, NAME_MAX);
		de[i].d_reclen = sizeof(struct dirent);
		de[i].d_namlen = (unsigned short)strnlen(de[i].d_name, NAME_MAX);

		i++;
		next = ((struct yaffs_obj *)iop->data1)->siblings.next;
		if(next == &obj->variant.dir_variant.children)
			iop->data1 = NULL; /* end of list */
		else
			iop->data1 = list_entry(next, struct yaffs_obj, siblings);
	}

	readlen = (ssize_t)(i * sizeof(struct dirent));
	iop->offset = iop->offset + readlen;

	yunlock(dev);

	return readlen;
}

static int ryfs_fstat(const rtems_filesystem_location_info_t *loc, struct stat *buf)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_location(loc);
	struct yaffs_dev *dev = obj->my_dev;
	rtems_yaffs_os_context *os_context = dev->os_context;

	ylock(dev);

	obj = yaffs_get_equivalent_obj(obj);
	if (obj != NULL) {
		buf->st_dev = os_context->dev;
		buf->st_ino = obj->obj_id;
		buf->st_mode = obj->yst_mode;
		buf->st_nlink = (nlink_t) yaffs_get_obj_link_count(obj);
		buf->st_rdev = obj->yst_rdev;
		buf->st_size = yaffs_get_obj_length(obj);
		buf->st_blksize = obj->my_dev->data_bytes_per_chunk;
		buf->st_blocks = (blkcnt_t)
			((buf->st_size + buf->st_blksize - 1) / buf->st_blksize);
		buf->st_uid = (uid_t) obj->yst_uid;
		buf->st_gid = (gid_t) obj->yst_gid;
		buf->st_atime = (time_t) obj->yst_atime;
		buf->st_ctime = (time_t) obj->yst_ctime;
		buf->st_mtime = (time_t) obj->yst_mtime;
	} else {
		errno = EIO;
		rv = -1;
	}

	yunlock(dev);

	return rv;
}

static int ryfs_fchmod(const rtems_filesystem_location_info_t *loc, mode_t mode)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_location(loc);
	int yc;

	obj = yaffs_get_equivalent_obj(obj);
	if (obj != NULL) {
		obj->yst_mode = mode;
		obj->dirty = 1;
		yc = yaffs_flush_file(obj, 0, 0, 0);
	} else {
		yc = YAFFS_FAIL;
	}

	if (yc != YAFFS_OK) {
		errno = EIO;
		rv = -1;
	}

	return rv;
}

static int ryfs_chown(
	const rtems_filesystem_location_info_t *loc,
	uid_t owner,
	gid_t group
)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_location(loc);
	int yc;

	obj = yaffs_get_equivalent_obj(obj);
	if (obj != NULL) {
		obj->yst_uid = owner;
		obj->yst_gid = group;
		obj->dirty = 1;
		yc = yaffs_flush_file(obj, 0, 0, 0);
	} else {
		yc = YAFFS_FAIL;
	}

	if (yc != YAFFS_OK) {
		errno = EIO;
		rv = -1;
	}

	return rv;
}

static int ryfs_fsync_or_fdatasync(rtems_libio_t *iop)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_iop(iop);
	struct yaffs_dev *dev = obj->my_dev;
	int yc;

	ylock(dev);
	yc = yaffs_flush_file(obj, 0, 1, 0);
	if (rtems_filesystem_location_is_instance_root(&iop->pathinfo)) {
		yaffs_flush_whole_cache(dev, 0);
	}
	yunlock(dev);

	if (yc != YAFFS_OK) {
		errno = EIO;
		rv = -1;
	}

	return rv;
}

static int ryfs_rmnod(
	const rtems_filesystem_location_info_t *parentloc,
	const rtems_filesystem_location_info_t *loc
)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_location(loc);
	int yc = yaffs_del_obj(obj);

	if (yc != YAFFS_OK) {
		errno = ENOTEMPTY;
		rv = -1;
	}

	return rv;
}

static int ryfs_file_open(rtems_libio_t *iop, const char *pathname, int oflag, mode_t mode)
{
	struct yaffs_obj *obj = ryfs_get_object_by_iop(iop);
	struct yaffs_dev *dev = obj->my_dev;
	int length = 0;

	ylock(dev);
	length = yaffs_get_obj_length(obj);
	if ((iop->flags & LIBIO_FLAGS_APPEND) != 0) {
		iop->offset = length;
	}
	yunlock(dev);

	return 0;
}

static int ryfs_file_close(rtems_libio_t *iop)
{
	struct yaffs_obj *obj = ryfs_get_object_by_iop(iop);
	struct yaffs_dev *dev = obj->my_dev;

	ylock(dev);
	yaffs_flush_file(obj, 1, 0, 1);
	yunlock(dev);

	return 0;
}

static ssize_t ryfs_file_read(rtems_libio_t *iop, void *buffer, size_t count)
{
	struct yaffs_obj *obj = ryfs_get_object_by_iop(iop);
	struct yaffs_dev *dev = obj->my_dev;
	ssize_t nr;
	int ol;
	size_t maxread;

	ylock(dev);

	ol = yaffs_get_obj_length(obj);
	if(iop->offset >= ol)
		maxread = 0;
	else
		maxread = (size_t)(ol - (int)iop->offset);
	if(count > maxread)
		count = maxread;

	nr = yaffs_file_rd(obj, buffer, iop->offset, (int)count);
	if (nr >= 0) {
		iop->offset += nr;
	} else {
		errno = EIO;
		nr = -1;
	}

	yunlock(dev);

	return nr;
}

static ssize_t ryfs_file_write(rtems_libio_t *iop, const void *buffer, size_t count)
{
	struct yaffs_obj *obj = ryfs_get_object_by_iop(iop);
	struct yaffs_dev *dev = obj->my_dev;
	ssize_t rv = -1;
	int max_size = INT_MAX;
	off_t offset;

	if (count == 0) {
		return 0;
	}

	ylock(dev);
	offset = iop->offset;
	if (offset < max_size) {
		size_t max_count = max_size - (size_t) offset;

		if (count > max_count) {
			count = max_count;
		}

		rv = yaffs_wr_file(obj, buffer, offset, (int) count, 0);
		if (rv > 0) {
			iop->offset += rv;
		} else {
			errno = ENOSPC;
			rv = -1;
		}
	} else {
		errno = EFBIG;
	}
	yunlock(dev);

	return rv;
}

static int ryfs_file_ftruncate(rtems_libio_t *iop, off_t length)
{
	int rv = 0;
	struct yaffs_obj *obj = ryfs_get_object_by_iop(iop);
	struct yaffs_dev *dev = obj->my_dev;
	int yc;

	ylock(dev);
	yc = yaffs_resize_file(obj, length);
	yunlock(dev);

	if (yc != YAFFS_OK) {
		errno = EIO;
		rv = -1;
	}

	return rv;
}

int rtems_yaffs_mount_handler(rtems_filesystem_mount_table_entry_t *mt_entry, const void *data)
{
	const rtems_yaffs_mount_data *mount_data = data;
	struct yaffs_dev *dev = mount_data->dev;

	if (dev->read_only && mt_entry->writeable) {
		errno = EACCES;
		return -1;
	}

	ylock(dev);
	if (yaffs_guts_initialise(dev) == YAFFS_FAIL) {
		yunlock(dev);
		errno = ENOMEM;
		return -1;
	}

	mt_entry->fs_info = dev;
	mt_entry->ops = &yaffs_ops;
	mt_entry->mt_fs_root->location.node_access = dev->root_dir;
	mt_entry->mt_fs_root->location.handlers = &yaffs_directory_handlers;

	yaffs_flush_whole_cache(dev, 0);
	yunlock(dev);

	return 0;
}

static void ryfs_fsunmount(rtems_filesystem_mount_table_entry_t *mt_entry)
{
	struct yaffs_dev *dev = ryfs_get_device_by_mt_entry(mt_entry);

	ylock(dev);
	yaffs_flush_whole_cache(dev, 1);
	yaffs_checkpoint_save(dev);
	yaffs_deinitialise(dev);
	yunlock(dev);
	rtems_yaffs_os_unmount(dev);
}

static void ryfs_lock(const rtems_filesystem_mount_table_entry_t *mt_entry)
{
	struct yaffs_dev *dev = ryfs_get_device_by_mt_entry(mt_entry);

	ylock(dev);
}

static void ryfs_unlock(const rtems_filesystem_mount_table_entry_t *mt_entry)
{
	struct yaffs_dev *dev = ryfs_get_device_by_mt_entry(mt_entry);

	yunlock(dev);
}

/**
 * Construct a link from parent/name to target.
 */
static int ryfs_symlink(const rtems_filesystem_location_info_t *parent_loc,
		const char *name,
		size_t namelen,
		const char *target)
{
	struct yaffs_obj *parent_dir = ryfs_get_object_by_location(parent_loc);
	struct yaffs_dev *dev = parent_dir->my_dev;
	uint32_t mode;
	struct yaffs_obj *created_link;
	int ret;

	ylock(dev);

	mode = S_IFLNK |
		((S_IRWXU | S_IRWXG | S_IRWXO) & ~rtems_filesystem_umask);

	created_link = yaffs_create_symlink(parent_dir, name, mode,
						geteuid(), getegid(), target);

	if (created_link != NULL) {
		ret = 0;
	} else {
		errno = EINVAL;
		ret = -1;
	}

	yunlock(dev);
	return ret;
}

/**
 * Read the target name of a symbolic link.  Interpretation of the path name is
 * up to the caller.
 *
 * @param loc The location of the symlink
 * @param dst_buf A non-NULL pointer to the caller's buffer for the characters.
 * @param dst_buf_size The size of the caller's buffer in characters.
 *
 * @retval -1 An error occurred, the error may be found via errno.
 * @retval non-negative size of the actual contents in characters, including the
 * terminating NULL.
 */
static ssize_t ryfs_readlink(const rtems_filesystem_location_info_t *loc,
		char *dst_buf, size_t dst_buf_size)
{
	struct yaffs_obj *link = ryfs_get_object_by_location(loc);
	struct yaffs_dev *dev = link->my_dev;

	ylock(dev);
	ssize_t chars_copied = -1;

	link = yaffs_get_equivalent_obj(link);
	if (!link) {
		errno = EBADF;
		goto error_locked;
	}

	if (link->variant_type != YAFFS_OBJECT_TYPE_SYMLINK) {
		errno = EINVAL;
		goto error_locked;
	}

	// Source string length including the terminating NULL.
	size_t src_buf_size = strlen(link->variant.symlink_variant.alias) + 1;
	if (src_buf_size > dst_buf_size)
		src_buf_size = dst_buf_size;
	memcpy(dst_buf, link->variant.symlink_variant.alias, src_buf_size);
	chars_copied = src_buf_size;

error_locked:
	yunlock(dev);
	return chars_copied;
}

static const rtems_filesystem_file_handlers_r yaffs_directory_handlers = {
	.open_h = rtems_filesystem_default_open,
	.close_h = rtems_filesystem_default_close,
	.read_h = ryfs_dir_read,
	.write_h = rtems_filesystem_default_write,
	.ioctl_h = rtems_filesystem_default_ioctl,
	.lseek_h = rtems_filesystem_default_lseek_directory,
	.fstat_h = ryfs_fstat,
	.ftruncate_h = rtems_filesystem_default_ftruncate_directory,
	.fsync_h = ryfs_fsync_or_fdatasync,
	.fdatasync_h = ryfs_fsync_or_fdatasync,
	.fcntl_h = rtems_filesystem_default_fcntl
};

static const rtems_filesystem_file_handlers_r yaffs_file_handlers = {
	.open_h = ryfs_file_open,
	.close_h = ryfs_file_close,
	.read_h = ryfs_file_read,
	.write_h = ryfs_file_write,
	.ioctl_h = rtems_filesystem_default_ioctl,
	.lseek_h = rtems_filesystem_default_lseek_file,
	.fstat_h = ryfs_fstat,
	.ftruncate_h = ryfs_file_ftruncate,
	.fsync_h = ryfs_fsync_or_fdatasync,
	.fdatasync_h = ryfs_fsync_or_fdatasync,
	.fcntl_h = rtems_filesystem_default_fcntl
};

static const rtems_filesystem_file_handlers_r yaffs_link_handlers = {
	.open_h = rtems_filesystem_default_open,
	.close_h = rtems_filesystem_default_close,
	.read_h = rtems_filesystem_default_read,
	.write_h = rtems_filesystem_default_write,
	.ioctl_h = rtems_filesystem_default_ioctl,
	.lseek_h = rtems_filesystem_default_lseek_file,
	.fstat_h = ryfs_fstat,
	.ftruncate_h = rtems_filesystem_default_ftruncate,
	.fsync_h = rtems_filesystem_default_fsync_or_fdatasync,
	.fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync,
	.fcntl_h = rtems_filesystem_default_fcntl,
};

static const rtems_filesystem_operations_table yaffs_ops = {
	.lock_h = ryfs_lock,
	.unlock_h = ryfs_unlock,
	.eval_path_h = ryfs_eval_path,
	.link_h = rtems_filesystem_default_link,
	.are_nodes_equal_h = rtems_filesystem_default_are_nodes_equal,
	.mknod_h = ryfs_mknod,
	.rmnod_h = ryfs_rmnod,
	.fchmod_h = ryfs_fchmod,
	.chown_h = ryfs_chown,
	.clonenod_h = rtems_filesystem_default_clonenode,
	.freenod_h = rtems_filesystem_default_freenode,
	.mount_h = rtems_filesystem_default_mount,
	.unmount_h = rtems_filesystem_default_unmount,
	.fsunmount_me_h = ryfs_fsunmount,
	.utime_h = ryfs_utime,
	.symlink_h = ryfs_symlink,
	.readlink_h = ryfs_readlink,
	.rename_h = ryfs_rename,
	.statvfs_h = rtems_filesystem_default_statvfs
};
