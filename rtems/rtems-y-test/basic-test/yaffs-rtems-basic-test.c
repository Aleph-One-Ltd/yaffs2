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


#define YPATH "/yaffs_mount_pt"





void set_uint8_t_buffer(uint8_t *buf, uint32_t n, uint8_t start, uint8_t inc)
{
	while (n) {
		*buf = start;
		buf++;
		start += inc;
		n--;
	}
}


#define FNAME YPATH"/test"
#define DIRNAME YPATH"/dirtest"

void dump_directory_tree_worker(const char *dname,int recursive)
{
	DIR *d;
	struct dirent *de;
	struct stat s;
	char str[1000];

	d = opendir(dname);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",dname,de->d_name);

			lstat(str,&s);

			printf("%s inode %d length %d mode %X ",
				str, (int)s.st_ino, (int)s.st_size, s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(readlink(str,str,100) < 0)
								printf("no alias");
							  else
								printf("\"%s\"",str);
							  break;
				default: printf("unknown"); break;
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
	dump_directory_tree_worker(dname,1);
}


void dumpDir(const char *dname)
{	dump_directory_tree_worker(dname,0);
}

int run_the_test(void)
{
	int fd;
	int ret;
	uint8_t buf[100];
	uint8_t buf2[100];


	dump_directory_tree(YPATH);

	ret = mkdir(DIRNAME, 0777);
	if (ret < 0)
		perror("mkdir "DIRNAME);

	fd = open(FNAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
	printf("open %s  = %d\n", FNAME, fd);
	if (fd < 0)
		perror("opening " FNAME);

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

