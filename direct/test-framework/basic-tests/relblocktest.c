/*
 * YAFFS: Yet another Flash File System. A NAND-flash specific file system.
 *
 * relblocktest: check param.relative_block_numbers.
 *
 * A partition that starts at block 16 of a device and uses relative block
 * numbers must have the same on-flash layout as the same partition starting
 * at block 0. So write a file system at block 16 with the flag set, copy
 * those blocks to the start of another image and mount that image without
 * the flag, with and without checkpoint.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "yaffsfs.h"
#include "yaffs_guts.h"
#include "yaffs_flexible_file_sim.h"

#define N_BLOCKS	192
#define PART_START	16
#define PART_BLOCKS	128
#define CHUNKS		64
#define CHUNK_SIZE	2048
#define SPARE_SIZE	64
#define FILE_SIZE	(300 * 1024)

#define IMG_A "relblock_a.img"
#define IMG_B "relblock_b.img"

int random_seed;
int simulate_power_failure;

static int failures;

#define CHECK(cond, msg) do { \
	if (!(cond)) { \
		printf("FAIL: %s\n", msg); \
		failures++; \
	} \
} while (0)

static void fill(unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		buf[i] = (unsigned char)(i * 7 + (i >> 8));
}

static void write_files(const char *root)
{
	unsigned char *buf = malloc(FILE_SIZE);
	char name[64];
	int fd;

	fill(buf, FILE_SIZE);

	sprintf(name, "%s/dir", root);
	CHECK(yaffs_mkdir(name, 0755) == 0, "mkdir");

	sprintf(name, "%s/dir/data", root);
	fd = yaffs_open(name, O_CREAT | O_RDWR, 0644);
	CHECK(fd >= 0, "create file");
	CHECK(yaffs_write(fd, buf, FILE_SIZE) == FILE_SIZE, "write file");
	yaffs_close(fd);
	free(buf);
}

static void verify_files(const char *root, const char *what)
{
	unsigned char *buf = malloc(FILE_SIZE);
	unsigned char *exp = malloc(FILE_SIZE);
	char name[64];
	int fd;

	fill(exp, FILE_SIZE);
	sprintf(name, "%s/dir/data", root);
	fd = yaffs_open(name, O_RDONLY, 0);
	if (fd < 0) {
		printf("FAIL: %s: open\n", what);
		failures++;
	} else {
		CHECK(yaffs_read(fd, buf, FILE_SIZE) == FILE_SIZE, what);
		CHECK(memcmp(buf, exp, FILE_SIZE) == 0, what);
		yaffs_close(fd);
	}
	free(buf);
	free(exp);
}

static int copy_blocks(void)
{
	size_t block_bytes = (size_t)CHUNKS * (CHUNK_SIZE + SPARE_SIZE);
	size_t total = block_bytes * PART_BLOCKS;
	char *buf = malloc(total);
	int in = open(IMG_A, O_RDONLY);
	int out = open(IMG_B, O_CREAT | O_TRUNC | O_WRONLY, 0644);
	int ok = buf && in >= 0 && out >= 0 &&
		 pread(in, buf, total, block_bytes * PART_START) == (ssize_t)total &&
		 write(out, buf, total) == (ssize_t)total;

	if (in >= 0)
		close(in);
	if (out >= 0)
		close(out);
	free(buf);
	return ok;
}

static struct yaffs_dev *make_dev(const char *name, const char *img,
				  int n_blocks, int start, int end)
{
	return yaffs_flexible_file_sim_create(name, img, 0, n_blocks,
					      start, end, CHUNKS, CHUNK_SIZE,
					      SPARE_SIZE);
}

static void remount_copy(const char *name, int skip_checkpt, const char *what)
{
	struct yaffs_dev *dev = make_dev(name, IMG_B, PART_BLOCKS, 0,
					 PART_BLOCKS - 1);

	if (!dev) {
		printf("FAIL: %s: create\n", what);
		failures++;
		return;
	}
	dev->param.skip_checkpt_rd = skip_checkpt;
	CHECK(yaffs_mount(name) >= 0, what);
	verify_files(name, what);
	yaffs_unmount(name);
}

int main(void)
{
	struct yaffs_dev *dev;

	unlink(IMG_A);
	dev = make_dev("rela", IMG_A, N_BLOCKS, PART_START,
		       PART_START + PART_BLOCKS - 1);
	if (!dev) {
		printf("FAIL: create device\n");
		return 1;
	}
	dev->param.relative_block_numbers = 1;

	CHECK(yaffs_mount("rela") >= 0, "mount partition");
	write_files("rela");
	CHECK(yaffs_unmount("rela") >= 0, "unmount partition");

	CHECK(yaffs_mount("rela") >= 0, "remount partition");
	verify_files("rela", "remount partition");
	yaffs_unmount("rela");

	CHECK(copy_blocks(), "copy partition blocks");
	remount_copy("relb", 0, "copy at block 0, checkpoint");
	remount_copy("relc", 1, "copy at block 0, full scan");

	unlink(IMG_A);
	unlink(IMG_B);
	printf(failures ? "relblocktest FAILED\n" : "relblocktest passed\n");
	return failures != 0;
}
