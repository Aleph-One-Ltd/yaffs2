/*
 *  Simple test program -- demonstrating use of IMFS
 */

#include <bsp.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <dirent.h>

#include <rtems/libio.h>
#include <yaffs/rtems_yaffs.h>

void set_uint8_t_buffer(uint8_t *buf, uint32_t n, uint8_t start, uint8_t inc)
{
	while (n) {
		*buf = start;
		buf++;
		start += inc;
		n--;
	}
}

void make_test_file_name(char *out, int out_size, char *root_path, char *dir, char *file, int index)
{
	if (index >= 0)
		snprintf(out, out_size, "%s/%s/%s-%d",
					root_path, dir, file, index);
	else
		snprintf(out, out_size, "%s/%s/%s",
					root_path, dir, file);
}

void make_test_dir_name(char *out, int out_size, char *root_path, char *dir)
{
	snprintf(out, out_size, "%s/%s", root_path, dir);
}

void dump_directory_tree_worker(const char *dname,int recursive)
{
	DIR *d;
	struct dirent *de;
	struct stat s;
	char str[1000];

	d = opendir(dname);

	if(!d) {
		printf("opendir failed\n");
	} else {
		while((de = readdir(d)) != NULL) {
			sprintf(str,"%s/%s",dname,de->d_name);

			lstat(str,&s);

			printf("%s inode %d length %d mode 0%o ",
				str, (int)s.st_ino, (int)s.st_size, s.st_mode);
			switch(s.st_mode & S_IFMT) {
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(readlink(str,str,100) < 0)
								printf("no alias");
							  else
								printf("\"%s\"",str);
							  break;
				default: printf("unknown mode"); break;
			}

			printf("\n");

			if((s.st_mode & S_IFMT) == S_IFDIR && recursive)
				dump_directory_tree_worker(str,1);
		}
		closedir(d);
	}
}

static void dump_directory_tree(const char *dname)
{
	printf("Directory tree of %s\n", dname);
	dump_directory_tree_worker(dname,1);
}


void recursively_delete(char *objname)
{
	struct stat s;
	DIR *d;
	struct dirent *de;
	char str[500];


	//printf("deleting %s\n", objname);
	lstat(objname, &s);

	switch(s.st_mode & S_IFMT) {
		case S_IFREG:
			printf("delete data file %s returns %d\n",
				objname, unlink(objname));
		break;
		case S_IFLNK:
			printf("delete symlink %s returns %d\n",
				objname, unlink(objname));
		break;
		case S_IFDIR:
			d = opendir(objname);
			if(!d) {
				printf("opendir failed\n");
			} else {
				while((de = readdir(d)) != NULL) {
					snprintf(str, sizeof(str), "%s/%s",
						objname, de->d_name);
					recursively_delete(str);
				}
				closedir(d);
			}
			printf("delete directory %s returns %d\n",
				objname, rmdir(objname));
		break;
	}
}



void dumpDir(const char *dname)
{
	dump_directory_tree_worker(dname,0);
}

int basic_file_test(char *root_path, char *test_path)
{
	char fname[100];
	char dname[100];
	int fd;
	int ret;
	uint8_t buf[100];
	uint8_t buf2[100];

	make_test_dir_name(dname, sizeof(dname), root_path, test_path);
	make_test_file_name(fname, sizeof(fname), root_path, test_path, "file", -1);

	ret = mkdir(dname, 0777);

	if (ret < 0) {
		perror("mkdir");
		return ret;
	}

	fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0777);
	printf("open %s  = %d\n", fname, fd);
	if (fd < 0) {
		perror("opening test file");
		return fd;
	}

	set_uint8_t_buffer(buf, sizeof(buf), 0xAA, 1);

	ret = write(fd, buf, sizeof(buf));

	printf("write returned %d\n", ret);

	if (ret < 0) {
		perror("writing file");
		return ret;
	}

	ret = fdatasync(fd);

	if (ret < 0) {
		perror("fdatasync problem");
		return ret;
	}

	ret = lseek(fd, 0, SEEK_END);

	printf("lseek end ret = %d\n", ret);

	ret = lseek(fd, 0, SEEK_SET);
	printf("lseek start ret = %d\n", ret);

	ret = read(fd, buf2, sizeof(buf2));

	printf("reading file ret = %d\n", ret);

	if (ret < 0) {
		perror("reading file");
		return ret;
	}

	dump_directory_tree(root_path);

	if (memcmp(buf, buf2, sizeof(buf)) == 0) {
		printf("buffers match\n");
		return 0;
	} else {
		printf("buffers do not match\n");
		return -1;
	}

	return ret;
}


int create_delete_files_pass(char *root_path, char *test_path, int n_files, int del_when_done)
{
	char fname[100];
	char lname[100];
	char dname[100];
	int *fds = NULL;
	int ret;
	int i;
	uint8_t buf[100];
	uint8_t buf2[100];

	fds = malloc(n_files * sizeof (int));

	if (!fds) {
		printf("Failed to malloc\n");
		ret = -1;
		goto err;
	}

	make_test_dir_name(dname, sizeof(dname), root_path, test_path);

	recursively_delete(dname);

	ret = access(dname, F_OK);
	printf("access of non-existing expects -1 returned %d\n", ret);

	if (ret != -1) {
		printf("access should have been -1, was %d\n", ret);
		ret = -1;
		goto err;
	}

	ret = mkdir(dname, 0777);

	if (ret < 0) {
		perror("mkdir");
		goto err;
	}

	ret = access(dname, F_OK);
	printf("access of existing returned %d\n", ret);

	if (ret < 0) {
		perror("access of existing directory");
		goto err;
	}

	for (i = 0; i < n_files; i++) {
		int link_fd;

		make_test_file_name(fname, sizeof(fname), root_path, test_path, "file-", i);
		make_test_file_name(lname, sizeof(lname), root_path, test_path, "link-", i);

		ret = symlink(fname, lname);

		if (ret < 0) {
			perror("creating symlink");
			goto err;
		}

		fds[i] = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0777);
		printf("open %s  = %d\n", fname, fds[i]);

		if (fds[i] < 0) {
			perror("opening test file");
			ret = fds[i];
			goto err;
		}


		link_fd = open(lname, O_RDWR, 0777);
		printf("opening link %s  = %d\n", lname, link_fd);

		if (link_fd < 0) {
			perror("opening symlink file");
			ret = link_fd;
			goto err;
		}
		close(link_fd);

	}

	set_uint8_t_buffer(buf, sizeof(buf), 0xAA, 1);

	for(i = 0; i < n_files; i++) {
		ret = write(fds[i], buf, sizeof(buf));
		printf("write returned %d\n", ret);
		if (ret < 0) {
			perror("writing file");
			goto err;
		}
	}

	for(i = 0; i < n_files; i++) {
		int trunc_size = sizeof(buf2)/2;

		ret = lseek(fds[i], 0, SEEK_END);

		printf("lseek end ret = %d\n", ret);

		ret = lseek(fds[i], 0, SEEK_SET);
		printf("lseek start ret = %d\n", ret);

		ret = read(fds[i], buf2, sizeof(buf2));

		printf("reading file ret = %d\n", ret);
		if (ret < 0) {
			perror("reading file");
			goto err;
		}
		ret = ftruncate(fds[i], trunc_size);

		if (ret < 0) {
			perror("ftruncate");
			goto err;
		}

		ret = lseek(fds[i], 0, SEEK_END);
		if (ret != trunc_size) {
			printf("truncated size is %d but lseek returned %d\n",
				trunc_size, ret);
			ret = -1;
			goto err;
		}


	}

	for(i = 0; i < n_files; i++) {
		ret = close(fds[i]);
		if (ret < 0) {
			perror("closing file");
			goto err;
		}
	}

	dump_directory_tree(root_path);

	if (memcmp(buf, buf2, sizeof(buf)) == 0) {
		printf("buffers match\n");
		ret = 0;
	} else {
		printf("buffers do not match\n");
		ret = -1;
	}

	if (del_when_done)
		recursively_delete(dname);
err:
	free(fds);

	return ret;
}

int create_delete_files(char *root_path, char *test_path, int n_files, int n_passes)
{
	int i;
	int ret;
	for (i = 0; i < n_passes; i++) {
		printf("\nCreate and Delete Files Pass %d\n", i);
		ret = create_delete_files_pass(root_path, test_path, n_files, 1);
		if (ret < 0)
			return ret;
	}
	return 0;
}

#define YPATH "/yaffs_mount_pt"
#define FNAME YPATH"/test"
#define DIRNAME YPATH"/dirtest"

void check_fail(int ret)
{
	if (ret < 0)
		printf("Test failed\n");
}

void run_the_test(void)
{
	check_fail(basic_file_test(YPATH, "basic-test-dir"));
	check_fail(create_delete_files(YPATH, "create-del-test-dir", 15, 50));

	printf("\n\n\nAll Yaffs Tests passed Ok\n\n\n");
}
