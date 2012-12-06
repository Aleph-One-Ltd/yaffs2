/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "yaffsfs.h"

#include "yaffs_guts.h" /* Only for dumping device innards */

extern int yaffs_trace_mask;

void dumpDir(const char *dname);

void copy_in_a_file(const char *yaffsName,const char *inName)
{
	int inh,outh;
	unsigned char buffer[100];
	int ni,no;
	inh = open(inName,O_RDONLY);
	outh = yaffs_open(yaffsName, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

	while((ni = read(inh,buffer,100)) > 0)
	{
		no = yaffs_write(outh,buffer,ni);
		if(ni != no)
		{
			printf("problem writing yaffs file\n");
		}

	}

	yaffs_close(outh);
	close(inh);
}

void make_a_file(const char *yaffsName,char bval,int sizeOfFile)
{
	int outh;
	int i;
	unsigned char buffer[100];

	outh = yaffs_open(yaffsName, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

	memset(buffer,bval,100);

	do{
		i = sizeOfFile;
		if(i > 100) i = 100;
		sizeOfFile -= i;

		yaffs_write(outh,buffer,i);

	} while (sizeOfFile > 0);


	yaffs_close(outh);

}

void make_pattern_file(char *fn,int size)
{
	int outh;
	int marker;
	int i;
	outh = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	yaffs_lseek(outh,size-1,SEEK_SET);
	yaffs_write(outh,"A",1);

	for(i = 0; i < size; i+=256)
	{
		marker = ~i;
		yaffs_lseek(outh,i,SEEK_SET);
		yaffs_write(outh,&marker,sizeof(marker));
	}
	yaffs_close(outh);

}

int check_pattern_file(char *fn)
{
	int h;
	int marker;
	int i;
	int size;
	int ok = 1;

	h = yaffs_open(fn, O_RDWR,0);
	size = yaffs_lseek(h,0,SEEK_END);

	for(i = 0; i < size; i+=256)
	{
		yaffs_lseek(h,i,SEEK_SET);
		yaffs_read(h,&marker,sizeof(marker));
		ok = (marker == ~i);
		if(!ok)
		{
		   printf("pattern check failed on file %s, size %d at position %d. Got %x instead of %x\n",
					fn,size,i,marker,~i);
		}
	}
	yaffs_close(h);
	return ok;
}





int dump_file_data(char *fn)
{
	int h;
	int i = 0;
	int ok = 1;
	unsigned char b;

	h = yaffs_open(fn, O_RDWR,0);


	printf("%s\n",fn);
	while(yaffs_read(h,&b,1)> 0)
	{
		printf("%02x",b);
		i++;
		if(i > 32)
		{
		   printf("\n");
		   i = 0;;
		 }
	}
	printf("\n");
	yaffs_close(h);
	return ok;
}



void dump_file(const char *fn)
{
	int i;
	int size;
	int h;

	h = yaffs_open(fn,O_RDONLY,0);
	if(h < 0)
	{
		printf("*****\nDump file %s does not exist\n",fn);
	}
	else
	{
		size = yaffs_lseek(h,0,SEEK_SET);
		printf("*****\nDump file %s size %d\n",fn,size);
		for(i = 0; i < size; i++)
		{

		}
	}
}

void create_file_of_size(const char *fn,int syze)
{
	int h;
	int n;
	int result;
	int iteration = 0;
	char xx[200];

	h = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

	while (syze > 0)
	{
		sprintf(xx,"%s %8d",fn,iteration);
		n = strlen(xx);
		result = yaffs_write(h,xx,n);
		if(result != n){
			printf("Wrote %d, should have been %d. syze is %d\n",result,n,syze);
			syze = 0;
		} else
			syze-=n;
		iteration++;
	}
	yaffs_close (h);
}

void verify_file_of_size(const char *fn,int syze)
{
	int h;
	int result;

	char xx[200];
	char yy[200];
	int l;

	int iterations = (syze + strlen(fn) -1)/ strlen(fn);

	h = yaffs_open(fn, O_RDONLY, S_IREAD | S_IWRITE);

	while (iterations > 0)
	{
		sprintf(xx,"%s %8d",fn,iterations);
		l = strlen(xx);

		result = yaffs_read(h,yy,l);
		yy[l] = 0;

		if(strcmp(xx,yy)){
			printf("=====>>>>> verification of file %s failed near position %lld\n",fn,(long long)yaffs_lseek(h,0,SEEK_CUR));
		}
		iterations--;
	}
	yaffs_close (h);
}

void create_resized_file_of_size(const char *fn,int syze1,int reSyze, int syze2)
{
	int h;

	int iterations;

	h = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

	iterations = (syze1 + strlen(fn) -1)/ strlen(fn);
	while (iterations > 0)
	{
		yaffs_write(h,fn,strlen(fn));
		iterations--;
	}

	yaffs_ftruncate(h,reSyze);

	yaffs_lseek(h,0,SEEK_SET);
	iterations = (syze2 + strlen(fn) -1)/ strlen(fn);
	while (iterations > 0)
	{
		yaffs_write(h,fn,strlen(fn));
		iterations--;
	}

	yaffs_close (h);
}


void do_some_file_stuff(const char *path)
{

	char fn[100];

	sprintf(fn,"%s/%s",path,"f1");
	create_file_of_size(fn,10000);

	sprintf(fn,"%s/%s",path,"fdel");
	create_file_of_size(fn,10000);
	yaffs_unlink(fn);

	sprintf(fn,"%s/%s",path,"f2");

	create_resized_file_of_size(fn,10000,3000,4000);
}

void yaffs_backward_scan_test(const char *path)
{
	char fn[100];

	yaffs_start_up();

	yaffs_mount(path);

	do_some_file_stuff(path);

	sprintf(fn,"%s/ddd",path);

	yaffs_mkdir(fn,0);

	do_some_file_stuff(fn);

	yaffs_unmount(path);

	yaffs_mount(path);
}

void null_name_test(const char *path)
{
	char fn[100];
	int h;
	yaffs_start_up();

	yaffs_mount(path);

	sprintf(fn,"%s",path);

	h = yaffs_open(fn,O_CREAT| O_TRUNC| O_RDWR, 0666);

	printf("%d\n",h);

}

char xxzz[2000];


void yaffs_device_flush_test(const char *path)
{
	char fn[100];
	int h;
	int i;

	yaffs_start_up();

	yaffs_mount(path);

	do_some_file_stuff(path);

	// Open and add some data to a few files
	for(i = 0; i < 10; i++) {

		sprintf(fn,"%s/ff%d",path,i);

		h = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IWRITE | S_IREAD);
		yaffs_write(h,xxzz,2000);
		yaffs_write(h,xxzz,2000);
	}
	yaffs_unmount(path);

	yaffs_mount(path);
}



void short_scan_test(const char *path, int fsize, int niterations)
{
	int i;
	char fn[100];

	sprintf(fn,"%s/%s",path,"f1");

	yaffs_start_up();
	for(i = 0; i < niterations; i++)
	{
		printf("\n*****************\nIteration %d\n",i);
		yaffs_mount(path);
		printf("\nmount: Directory look-up of %s\n",path);
		dumpDir(path);
		make_a_file(fn,1,fsize);
		yaffs_unmount(path);
	}
}



void scan_pattern_test(const char *path, int fsize, int niterations)
{
	int i;
	int j;
	char fn[3][100];
	int result;

	sprintf(fn[0],"%s/%s",path,"f0");
	sprintf(fn[1],"%s/%s",path,"f1");
	sprintf(fn[2],"%s/%s",path,"f2");

	yaffs_start_up();

	for(i = 0; i < niterations; i++)
	{
		printf("\n*****************\nIteration %d\n",i);
		yaffs_mount(path);
		printf("\nmount: Directory look-up of %s\n",path);
		dumpDir(path);
		for(j = 0; j < 3; j++)
		{
			result = dump_file_data(fn[j]);
			result = check_pattern_file(fn[j]);
			make_pattern_file(fn[j],fsize);
			result = dump_file_data(fn[j]);
			result = check_pattern_file(fn[j]);
		}
		yaffs_unmount(path);
	}
}

void fill_disk(const char *path,int nfiles)
{
	int h;
	int n;
	int result;
	int f;

        static char xx[600];
	char str[50];

	for(n = 0; n < nfiles; n++)
	{
		sprintf(str,"%s/%d",path,n);

		h = yaffs_open(str, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

		printf("writing file %s handle %d ",str, h);

		while ((result = yaffs_write(h,xx,600)) == 600)
		{
			f = yaffs_freespace(path);
		}
		result = yaffs_close(h);
		printf(" close %d\n",result);
	}
}

void fill_disk_and_delete(const char *path, int nfiles, int ncycles)
{
	int i,j;
	char str[50];
	int result;

	for(i = 0; i < ncycles; i++)
	{
		printf("@@@@@@@@@@@@@@ cycle %d\n",i);
		fill_disk(path,nfiles);

		for(j = 0; j < nfiles; j++)
		{
			sprintf(str,"%s/%d",path,j);
			result = yaffs_unlink(str);
			printf("unlinking file %s, result %d\n",str,result);
		}
	}
}


void fill_files(const char *path,int flags, int maxIterations,int siz)
{
	int i;
	int j;
	char str[50];
	int h;

	i = 0;

	do{
		sprintf(str,"%s/%d",path,i);
		h = yaffs_open(str, O_CREAT | O_TRUNC | O_RDWR,S_IREAD | S_IWRITE);

		if(h >= 0)
		{
			for(j = 0; j < siz; j++)
			{
				yaffs_write(h,str,1);
			}
		}

		if( flags & 1)
		{
			yaffs_unlink(str);
		}
		i++;
	} while(h >= 0 && i < maxIterations);

	if(flags & 2)
	{
		i = 0;
		do{
			sprintf(str,"%s/%d",path,i);
			printf("unlink %s\n",str);
			i++;
		} while(yaffs_unlink(str) >= 0);
	}
}

void leave_unlinked_file(const char *path,int maxIterations,int siz)
{
	int i;
	char str[50];
	int h;

	i = 0;

	do{
		sprintf(str,"%s/%d",path,i);
		printf("create %s\n",str);
		h = yaffs_open(str, O_CREAT | O_TRUNC | O_RDWR,S_IREAD | S_IWRITE);
		if(h >= 0)
		{
			yaffs_unlink(str);
		}
		i++;
	} while(h < 0 && i < maxIterations);

	if(h >= 0)
	{
		for(i = 0; i < siz; i++)
		{
			yaffs_write(h,str,1);
		}
	}

	printf("Leaving file %s open\n",str);

}

void dumpDirFollow(const char *dname)
{
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];

	d = yaffs_opendir(dname);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",dname,de->d_name);

			yaffs_lstat(str,&s);

			printf("%s ino %lld length %d mode %X ",de->d_name,(int)s.st_ino,s.st_size,s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(yaffs_readlink(str,str,100) < 0)
								printf("no alias");
							  else
								printf("\"%s\"",str);
							  break;
				default: printf("unknown"); break;
			}

			printf("\n");
		}

		yaffs_closedir(d);
	}
	printf("\n");

	printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));

}


void dump_directory_tree_worker(const char *dname,int recursive)
{
	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;
	char str[1000];

	d = yaffs_opendir(dname);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",dname,de->d_name);

			yaffs_lstat(str,&s);

			printf("%s inode %d obj %x length %lld mode %X ",
				str,s.st_ino,de->d_dont_use, s.st_size,s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(yaffs_readlink(str,str,100) < 0)
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

		yaffs_closedir(d);
	}

}

static void dump_directory_tree(const char *dname)
{
	dump_directory_tree_worker(dname,1);
	printf("\n");
	printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));
}

void dumpDir(const char *dname)
{	dump_directory_tree_worker(dname,0);
	printf("\n");
	printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));
}


static void PermissionsCheck(const char *path, mode_t tmode, int tflags,int expectedResult)
{
	int fd;

	if(yaffs_chmod(path,tmode)< 0) printf("chmod failed\n");

	fd = yaffs_open(path,tflags,0);

	if((fd >= 0) != (expectedResult > 0))
	{
		printf("Permissions check %x %x %d failed\n",tmode,tflags,expectedResult);
	}
	else
	{
		printf("Permissions check %x %x %d OK\n",tmode,tflags,expectedResult);
	}


	yaffs_close(fd);


}

int long_test(int argc, char *argv[])
{

	int f;
	int r;
	char buffer[20];

	char str[100];

	int h;
	mode_t temp_mode;
	struct yaffs_stat ystat;

	yaffs_start_up();

	yaffs_mount("/boot");
	yaffs_mount("/data");
	yaffs_mount("/flash");
	yaffs_mount("/ram");

	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /data\n");
	dumpDir("/data");
	printf("\nDirectory look-up of /flash\n");
	dumpDir("/flash");

	//leave_unlinked_file("/flash",20000,0);
	//leave_unlinked_file("/data",20000,0);

	leave_unlinked_file("/ram",20,0);


	f = yaffs_open("/boot/b1", O_RDONLY,0);

	printf("open /boot/b1 readonly, f=%d\n",f);

	f = yaffs_open("/boot/b1", O_CREAT,S_IREAD | S_IWRITE);

	printf("open /boot/b1 O_CREAT, f=%d\n",f);


	r = yaffs_write(f,"hello",1);
	printf("write %d attempted to write to a read-only file\n",r);

	r = yaffs_close(f);

	printf("close %d\n",r);

	f = yaffs_open("/boot/b1", O_RDWR,0);

	printf("open /boot/b1 O_RDWR,f=%d\n",f);


	r = yaffs_write(f,"hello",2);
	printf("write %d attempted to write to a writeable file\n",r);
	r = yaffs_write(f,"world",3);
	printf("write %d attempted to write to a writeable file\n",r);

	r= yaffs_lseek(f,0,SEEK_END);
	printf("seek end %d\n",r);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);
	r= yaffs_lseek(f,0,SEEK_SET);
	printf("seek set %d\n",r);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);

	// Check values reading at end.
	// A read past end of file should return 0 for 0 bytes read.

	r= yaffs_lseek(f,0,SEEK_END);
	r = yaffs_read(f,buffer,10);
	printf("read at end returned  %d\n",r);
	r= yaffs_lseek(f,500,SEEK_END);
	r = yaffs_read(f,buffer,10);
	printf("read past end returned  %d\n",r);

	r = yaffs_close(f);

	printf("close %d\n",r);

	copy_in_a_file("/boot/yyfile","xxx");

	// Create a file with a long name

	copy_in_a_file("/boot/file with a long name","xxx");


	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	// Check stat
	r = yaffs_lstat("/boot/file with a long name",&ystat);

	// Check rename

	r = yaffs_rename("/boot/file with a long name","/boot/r1");

	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	// Check unlink
	r = yaffs_unlink("/boot/r1");

	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	// Check mkdir

	r = yaffs_mkdir("/boot/directory1",0);

	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

	// add a file to the directory
	copy_in_a_file("/boot/directory1/file with a long name","xxx");

	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

	//  Attempt to delete directory (should fail)

	r = yaffs_rmdir("/boot/directory1");

	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

	// Delete file first, then rmdir should work
	r = yaffs_unlink("/boot/directory1/file with a long name");
	r = yaffs_rmdir("/boot/directory1");


	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

#if 0
	fill_disk_and_delete("/boot",20,20);

	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
#endif

	yaffs_symlink("yyfile","/boot/slink");

	yaffs_readlink("/boot/slink",str,100);
	printf("symlink alias is %s\n",str);




	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot (using stat instead of lstat)\n");
	dumpDirFollow("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

	h = yaffs_open("/boot/slink",O_RDWR,0);

	printf("file length is %d\n",(int)yaffs_lseek(h,0,SEEK_END));

	yaffs_close(h);

	yaffs_unlink("/boot/slink");


	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	// Check chmod

	yaffs_lstat("/boot/yyfile",&ystat);
	temp_mode = ystat.st_mode;

	yaffs_chmod("/boot/yyfile",0x55555);
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	yaffs_chmod("/boot/yyfile",temp_mode);
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	// Permission checks...
	PermissionsCheck("/boot/yyfile",0, O_WRONLY,0);
	PermissionsCheck("/boot/yyfile",0, O_RDONLY,0);
	PermissionsCheck("/boot/yyfile",0, O_RDWR,0);

	PermissionsCheck("/boot/yyfile",S_IREAD, O_WRONLY,0);
	PermissionsCheck("/boot/yyfile",S_IREAD, O_RDONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD, O_RDWR,0);

	PermissionsCheck("/boot/yyfile",S_IWRITE, O_WRONLY,1);
	PermissionsCheck("/boot/yyfile",S_IWRITE, O_RDONLY,0);
	PermissionsCheck("/boot/yyfile",S_IWRITE, O_RDWR,0);

	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_WRONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_RDONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_RDWR,1);

	yaffs_chmod("/boot/yyfile",temp_mode);

	//create a zero-length file and unlink it (test for scan bug)

	h = yaffs_open("/boot/zlf",O_CREAT | O_TRUNC | O_RDWR,0);
	yaffs_close(h);

	yaffs_unlink("/boot/zlf");


	yaffs_dump_dev("/boot");

	fill_disk_and_delete("/boot",20,20);

	yaffs_dump_dev("/boot");

	fill_files("/boot",1,10000,0);
	fill_files("/boot",1,10000,5000);
	fill_files("/boot",2,10000,0);
	fill_files("/boot",2,10000,5000);

	leave_unlinked_file("/data",20000,0);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);

	yaffs_dump_dev("/boot");
	yaffs_dump_dev("/data");



	return 0;

}

int huge_directory_test_on_path(char *path)
{

	yaffs_DIR *d;
	struct yaffs_dirent *de;
	struct yaffs_stat s;

	int f;
	int i;

	int total = 0;
	int lastTotal = 0;

	char str[100];


	yaffs_start_up();

	yaffs_mount(path);

	// Create a large number of files

	for(i = 0; i < 2000; i++)
	{
	  sprintf(str,"%s/%d",path,i);

	   f = yaffs_open(str,O_CREAT,S_IREAD | S_IWRITE);
	   yaffs_close(f);
	}



	d = yaffs_opendir(path);
	i = 0;
	if (d) {
	while((de = yaffs_readdir(d)) != NULL) {
	if (total >lastTotal+100*9*1024||(i & 1023)==0){
	printf("files = %d, total = %d\n",i, total);
	lastTotal = total;
	}
		i++;
		sprintf(str,"%s/%s",path,de->d_name);
		yaffs_lstat(str,&s);
		switch(s.st_mode & S_IFMT){
		case S_IFREG:
	//printf("data file");
	total += s.st_size;
	break;
	}
	}

	yaffs_closedir(d);
	}

	return 0;
}

int yaffs_scan_test(const char *path)
{
	return 0;
}


void rename_over_test(const char *mountpt)
{
	int i;
	char a[100];
	char b[100];
	char c[100];

	sprintf(a,"%s/a",mountpt);
	sprintf(b,"%s/b",mountpt);
	sprintf(c,"%s/c",mountpt);

	yaffs_start_up();

	yaffs_mount(mountpt);

	printf("Existing files\n");
	dumpDirFollow(mountpt);



	i = yaffs_open(c,O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
	printf("File c handle is %d\n",i);
	yaffs_close(i);
	i = yaffs_open(a,O_CREAT | O_TRUNC | O_RDWR,  S_IREAD | S_IWRITE);
	yaffs_close(i);
	i = yaffs_open(b,O_CREAT | O_TRUNC | O_RDWR,  S_IREAD | S_IWRITE);
	yaffs_close(i);
	yaffs_rename(a,b); // rename over
	yaffs_rename(b,a); // rename back again (not renaimng over)
	yaffs_rename(a,b); // rename back again (not renaimng over)


	yaffs_unmount(mountpt);

}


int resize_stress_test(const char *path)
{
   int a,b,i,j;
   int x;
   int r;
   char aname[100];
   char bname[100];

   char abuffer[1000];
   char bbuffer[1000];

   yaffs_start_up();

   yaffs_mount(path);

   sprintf(aname,"%s%s",path,"/a");
   sprintf(bname,"%s%s",path,"/b");

   memset(abuffer,'a',1000);
   memset(bbuffer,'b',1000);

   a = yaffs_open(aname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   b = yaffs_open(bname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

   printf(" %s %d %s %d\n",aname,a,bname,b);

   x = 0;

   for(j = 0; j < 100; j++)
   {
		yaffs_lseek(a,0,SEEK_END);


		for(i = 0; i <20000; i++)
		{
		   //r =        yaffs_lseek(b,i,SEEK_SET);
			//r = yaffs_write(b,bbuffer,1000);

			if(x & 0x16)
			{
				// shrink
				int syz = yaffs_lseek(a,0,SEEK_END);

				syz -= 500;
				if(syz < 0) syz = 0;
				yaffs_ftruncate(a,syz);

			}
			else
			{
				//expand
				r = yaffs_lseek(a,i * 500,SEEK_SET);
				r = yaffs_write(a,abuffer,1000);
			}
			x++;

		}
   }

   return 0;

}


int overwrite_test(const char *path)
{
   char aname[100];
   char bname[100];
   int i;
   int j;
   int a;
   int b;
   yaffs_start_up();

   yaffs_mount(path);

   sprintf(aname,"%s%s",path,"/a");
   sprintf(bname,"%s%s",path,"/b");

   b = yaffs_open(bname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   for(j= 0; j < 500; j++){
   	yaffs_write(b,bname,100);
	a = yaffs_open(aname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   	for(i = 0; i < rand() % 20000; i++)
   		yaffs_write(a,&a,sizeof(a));
	yaffs_close(a);
   }

   return 0;

}


int root_perm_remount(const char *path)
{
   struct yaffs_stat s;

   yaffs_start_up();

   yaffs_mount(path);

   yaffs_lstat(path,&s);
   printf("root perms after mount %x\n",s.st_mode);

   yaffs_chmod(path, 0777);

   yaffs_lstat(path,&s);
   printf("root perms after setting to 0777 is  %x\n",s.st_mode);

   yaffs_unmount(path);

   return 0;

}


int resize_stress_test_no_grow_complex(const char *path,int iters)
{
   int a,b,i,j;
   int x;
   int r;
   char aname[100];
   char bname[100];

   char abuffer[1000];
   char bbuffer[1000];


   yaffs_start_up();

   yaffs_mount(path);

   sprintf(aname,"%s%s",path,"/a");
   sprintf(bname,"%s%s",path,"/b");

   memset(abuffer,'a',1000);
   memset(bbuffer,'b',1000);

   a = yaffs_open(aname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   b = yaffs_open(bname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

   printf(" %s %d %s %d\n",aname,a,bname,b);

   x = 0;

   for(j = 0; j < iters; j++)
   {
		yaffs_lseek(a,0,SEEK_END);


		for(i = 0; i <20000; i++)
		{
		   //r =        yaffs_lseek(b,i,SEEK_SET);
			//r = yaffs_write(b,bbuffer,1000);

			if(!(x%20))
			{
				// shrink
				int syz = yaffs_lseek(a,0,SEEK_END);

				while(syz > 4000)
				{

					syz -= 2050;
					if(syz < 0) syz = 0;
					yaffs_ftruncate(a,syz);
					syz = yaffs_lseek(a,0,SEEK_END);
					printf("shrink to %d\n",syz);
				}


			}
			else
			{
				//expand
				r = yaffs_lseek(a,500,SEEK_END);
				r = yaffs_write(a,abuffer,1000);
			}
			x++;


		}

		printf("file size is %lld\n",(long long)yaffs_lseek(a,0,SEEK_END));

   }

   return 0;

}

int resize_stress_test_no_grow(const char *path,int iters)
{
   int a,b,i,j;
   int x;
   int r;
   char aname[100];
   char bname[100];

   char abuffer[1000];
   char bbuffer[1000];

   yaffs_start_up();

   yaffs_mount(path);

   sprintf(aname,"%s%s",path,"/a");
   sprintf(bname,"%s%s",path,"/b");

   memset(abuffer,'a',1000);
   memset(bbuffer,'b',1000);

   a = yaffs_open(aname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   b = yaffs_open(bname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

   printf(" %s %d %s %d\n",aname,a,bname,b);

   x = 0;

   for(j = 0; j < iters; j++)
   {
		yaffs_lseek(a,0,SEEK_END);


		for(i = 0; i <20000; i++)
		{
		   //r =        yaffs_lseek(b,i,SEEK_SET);
			//r = yaffs_write(b,bbuffer,1000);

			if(!(x%20))
			{
				// shrink
				int syz = yaffs_lseek(a,0,SEEK_END);

				while(syz > 4000)
				{

					syz -= 2050;
					if(syz < 0) syz = 0;
					yaffs_ftruncate(a,syz);
					syz = yaffs_lseek(a,0,SEEK_END);
					printf("shrink to %d\n",syz);
				}


			}
			else
			{
				//expand
				r = yaffs_lseek(a,-500,SEEK_END);
				r = yaffs_write(a,abuffer,1000);
			}
			x++;


		}
		printf("file size is %lld\n",(long long)yaffs_lseek(a,0,SEEK_END));

   }

   return 0;

}

int directory_rename_test(void)
{
	int r;
	yaffs_start_up();

	yaffs_mount("/ram");
	yaffs_mkdir("/ram/a",0);
	yaffs_mkdir("/ram/a/b",0);
	yaffs_mkdir("/ram/c",0);

	printf("\nDirectory look-up of /ram\n");
	dumpDir("/ram");
	dumpDir("/ram/a");
	dumpDir("/ram/a/b");

	printf("Do rename (should fail)\n");

	r = yaffs_rename("/ram/a","/ram/a/b/d");
	printf("\nDirectory look-up of /ram\n");
	dumpDir("/ram");
	dumpDir("/ram/a");
	dumpDir("/ram/a/b");

	printf("Do rename (should not fail)\n");

	r = yaffs_rename("/ram/c","/ram/a/b/d");
	printf("\nDirectory look-up of /ram\n");
	dumpDir("/ram");
	dumpDir("/ram/a");
	dumpDir("/ram/a/b");


	return 1;

}

int cache_read_test(void)
{
	int a,b,c;
	int i;
	int sizeOfFiles = 500000;
	char buffer[100];

	yaffs_start_up();

	yaffs_mount("/boot");

	make_a_file("/boot/a",'a',sizeOfFiles);
	make_a_file("/boot/b",'b',sizeOfFiles);

	a = yaffs_open("/boot/a",O_RDONLY,0);
	b = yaffs_open("/boot/b",O_RDONLY,0);
	c = yaffs_open("/boot/c", O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

	do{
		i = sizeOfFiles;
		if (i > 100) i = 100;
		sizeOfFiles  -= i;
		yaffs_read(a,buffer,i);
		yaffs_read(b,buffer,i);
		yaffs_write(c,buffer,i);
	} while(sizeOfFiles > 0);



	return 1;

}

int cache_bypass_bug_test(void)
{
	// This test reporoduces a bug whereby YAFFS caching *was* buypassed
	// resulting in erroneous reads after writes.
	// This bug has been fixed.

	int a;
	char buffer1[1000];
	char buffer2[1000];

	memset(buffer1,0,sizeof(buffer1));
	memset(buffer2,0,sizeof(buffer2));

	yaffs_start_up();

	yaffs_mount("/boot");

	// Create a file of 2000 bytes.
	make_a_file("/boot/a",'X',2000);

	a = yaffs_open("/boot/a",O_RDWR, S_IREAD | S_IWRITE);

	// Write a short sequence to the file.
	// This will go into the cache.
	yaffs_lseek(a,0,SEEK_SET);
	yaffs_write(a,"abcdefghijklmnopqrstuvwxyz",20);

	// Read a short sequence from the file.
	// This will come from the cache.
	yaffs_lseek(a,0,SEEK_SET);
	yaffs_read(a,buffer1,30);

	// Read a page size sequence from the file.
	yaffs_lseek(a,0,SEEK_SET);
	yaffs_read(a,buffer2,512);

	printf("buffer 1 %s\n",buffer1);
	printf("buffer 2 %s\n",buffer2);

	if(strncmp(buffer1,buffer2,20))
	{
		printf("Cache bypass bug detected!!!!!\n");
	}


	return 1;
}


int free_space_check(void)
{
	int f;

		yaffs_start_up();
		yaffs_mount("/boot");
	    fill_disk("/boot/",2);
	    f = yaffs_freespace("/boot");

	    printf("%d free when disk full\n",f);
	    return 1;
}

int truncate_test(void)
{
	int a;
	int r;
	int i;
	int l;

	char y[10];

	yaffs_start_up();
	yaffs_mount("/boot");

	yaffs_unlink("/boot/trunctest");

	a = yaffs_open("/boot/trunctest", O_CREAT | O_TRUNC | O_RDWR,  S_IREAD | S_IWRITE);

	yaffs_write(a,"abcdefghijklmnopqrstuvwzyz",26);

	yaffs_ftruncate(a,3);
	l= yaffs_lseek(a,0,SEEK_END);

	printf("truncated length is %d\n",l);

	yaffs_lseek(a,5,SEEK_SET);
	yaffs_write(a,"1",1);

	yaffs_lseek(a,0,SEEK_SET);

	r = yaffs_read(a,y,10);

	printf("read %d bytes:",r);

	for(i = 0; i < r; i++) printf("[%02X]",y[i]);

	printf("\n");

	return 0;

}





void fill_disk_test(const char *mountpt)
{
	int i;
	yaffs_start_up();

	for(i = 0; i < 5; i++)
	{
		yaffs_mount(mountpt);
		fill_disk_and_delete(mountpt,100,i+1);
		yaffs_unmount(mountpt);
	}

}


void fill_files_test(const char *mountpt)
{
	int i;
	yaffs_start_up();

	for(i = 0; i < 5; i++)
	{
		yaffs_mount(mountpt);
		fill_files(mountpt,2,3,100);
		yaffs_unmount(mountpt);
	}

}

void fill_empty_files_test(const char *mountpt)
{
	int i;
	yaffs_start_up();
	char name[100];
	int result = 0;

	int d,f;

	for(i = 0; i < 5; i++)
	{
		yaffs_mount(mountpt);
		for(d = 0; result >= 0 && d < 1000; d++){
			sprintf(name,"%s/%d",mountpt,d);
			result= yaffs_mkdir(name,0);
			printf("creating directory %s result %d\n",name,result);

			for(f = 0; result >= 0 && f < 100; f++){
				sprintf(name,"%s/%d/%d",mountpt,d,f);
				result= yaffs_open(name,O_CREAT, 0);
				yaffs_close(result);
				printf("creating file %s result %d\n",name,result);
			}
		}
		yaffs_unmount(mountpt);
	}

}

void long_name_test(const char *mountpt)
{
	int i;
	yaffs_start_up();
	char fullName[1000];
	char name[300];
	int result = 0;

	int f;

	// Make a 256 byte name
	memset(name,0,sizeof(name));
	for(i = 0; i < 256; i++)
		name[i] = '0' + i % 10;

	sprintf(fullName,"%s/%s",mountpt,name);

	for(i = 0; i < 1; i++)
	{
		yaffs_mount(mountpt);

		printf("Files at start\n");
		dumpDir(mountpt);

		printf("Creating file %s\n",fullName);

		f = yaffs_open(fullName,O_CREAT | O_RDWR,0);
		yaffs_close(f);

		printf("Result %d\n",f);

		printf("Files\n");
		dumpDir(mountpt);

		printf("Deleting %s\n",fullName);
		result = yaffs_unlink(fullName);
		printf("Result %d\n",result);

		printf("Files\n");

		dumpDir(mountpt);

		yaffs_unmount(mountpt);
	}

}


void lookup_test(const char *mountpt)
{
	int i;
	int h;
	char a[100];


	yaffs_DIR *d;
	struct yaffs_dirent *de;

	yaffs_start_up();

	yaffs_mount(mountpt);

	d = yaffs_opendir(mountpt);

	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{

		for(i = 0; (de = yaffs_readdir(d)) != NULL; i++)
		{
			printf("unlinking %s\n",de->d_name);
			yaffs_unlink(de->d_name);
		}

		printf("%d files deleted\n",i);
	}


	for(i = 0; i < 2000; i++){
	sprintf(a,"%s/%d",mountpt,i);
		h =  yaffs_open(a,O_CREAT | O_TRUNC | O_RDWR, 0);
		yaffs_close(h);
	}

	yaffs_rewinddir(d);
	for(i = 0; (de = yaffs_readdir(d)) != NULL; i++)
	{
		printf("%d  %s\n",i,de->d_name);
	}

	printf("%d files listed\n\n\n",i);

	yaffs_rewinddir(d);
	yaffs_readdir(d);
	yaffs_readdir(d);
	yaffs_readdir(d);

	for(i = 0; i < 2000; i++){
		sprintf(a,"%s/%d",mountpt,i);
		yaffs_unlink(a);
	}


	yaffs_unmount(mountpt);

}

void link_test0(const char *mountpt)
{
	char namea[300];
	char nameb[300];
	int result = 0;


	yaffs_start_up();
	yaffs_mount(mountpt);


	sprintf(namea,"%s/a",mountpt);
	sprintf(nameb,"%s/b",mountpt);

	printf("mounted\n");
        dumpDir(mountpt);

	yaffs_unlink(namea);
	printf("a unlinked\n");
        dumpDir(mountpt);

	yaffs_unlink(nameb);
	printf("b unlinked\n");
        dumpDir(mountpt);

	result = yaffs_open(namea,O_CREAT| O_RDWR,0666);
        yaffs_close(result);
	printf("a created\n");
        dumpDir(mountpt);

        yaffs_link(namea,nameb);
        printf("linked\n");
        dumpDir(mountpt);
        yaffs_unlink(namea);
        printf("a ulinked\n");
        dumpDir(mountpt);
        yaffs_unlink(nameb);
        printf("b unlinked\n");
        dumpDir(mountpt);

	yaffs_unmount(mountpt);
}


void link_test1(const char *mountpt)
{
	int i;
	int h;
	char a[100];
	char b[100];
	char c[100];

	sprintf(a,"%s/aaa",mountpt);
	sprintf(b,"%s/bbb",mountpt);
	sprintf(c,"%s/ccc",mountpt);

	yaffs_start_up();

	yaffs_mount(mountpt);


	h = yaffs_open(a, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
	for(i = 0; i < 100; i++)
		yaffs_write(h,a,100);

	yaffs_close(h);

	yaffs_unlink(b);
	yaffs_unlink(c);
	yaffs_link(a,b);
	yaffs_link(a,c);
	yaffs_unlink(b);
	yaffs_unlink(c);
	yaffs_unlink(a);


	yaffs_unmount(mountpt);
	yaffs_mount(mountpt);

	printf("link test done\n");
}

void handle_test(const char *mountpt)
{
	int i;
	int h;
	int cycle;
	char a[100];

	sprintf(a,"%s/aaa",mountpt);

	yaffs_start_up();

	yaffs_mount(mountpt);

        for(cycle = 0; cycle < 5; cycle++){
        printf("Start cycle %d\n",cycle);
 	i = 0;
	do {
        h = yaffs_open(a, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
	printf("%d  handle %d\n",i,h);
	i++;
	} while(h >= 0);

	while(i >= -1) {
	 yaffs_close(i);
	 i--;
        }
        }

	yaffs_unmount(mountpt);
}

void freespace_test(const char *mountpt)
{
	int i;
	int h;
	char a[100];

	int  f0;
	int f1;
	int f2;
	int f3;
	sprintf(a,"%s/aaa",mountpt);

	yaffs_start_up();

	yaffs_mount(mountpt);

	f0 = yaffs_freespace(mountpt);

	h = yaffs_open(a, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

	for(i = 0; i < 100; i++)
		yaffs_write(h,a,100);

	yaffs_close(h);

	f1 = yaffs_freespace(mountpt);

	yaffs_unlink(a);

	f2 = yaffs_freespace(mountpt);


	yaffs_unmount(mountpt);
	yaffs_mount(mountpt);

	f3 = yaffs_freespace(mountpt);

	printf("%d\n%d\n%d\n%d\n",f0, f1,f2,f3);


}

void simple_rw_test(const char *mountpt)
{
	int i;
	int h;
	char a[100];

	int x;
	int result;

	sprintf(a,"%s/aaa",mountpt);

	yaffs_start_up();

	yaffs_mount(mountpt);

	yaffs_unlink(a);

	h = yaffs_open(a,O_CREAT| O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

	for(i = 100000;i < 200000; i++){
		result = yaffs_write(h,&i,sizeof(i));

		if(result != 4)
		{
			printf("write error\n");
			exit(1);
		}
	}

	//yaffs_close(h);

	// h = yaffs_open(a,O_RDWR, S_IREAD | S_IWRITE);


	yaffs_lseek(h,0,SEEK_SET);

	for(i = 100000; i < 200000; i++){
		result = yaffs_read(h,&x,sizeof(x));

		if(result != 4 || x != i){
			printf("read error %d %x %x\n",i,result,x);
		}
	}

	printf("Simple rw test passed\n");



}


void scan_deleted_files_test(const char *mountpt)
{
	char fn[100];
	char sub[100];

	const char *p;

	int i;
	int j;
	int k;
	int h;

	sprintf(sub,"%s/sdir",mountpt);
	yaffs_start_up();

	for(j = 0; j < 10; j++)
	{
		printf("\n\n>>>>>>> Run %d <<<<<<<<<<<<<\n\n",j);
		yaffs_mount(mountpt);
		yaffs_mkdir(sub,0);


		p = (j & 0) ? mountpt: sub;

		for(i = 0; i < 100; i++)
		{
		  sprintf(fn,"%s/%d",p,i);

		  if(i & 1)
		  {
			  h = yaffs_open(fn,O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
			  for(k = 0; k < 1000; k++)
				  yaffs_write(h,fn,100);
			  yaffs_close(h);
		  }
		  else
		    	yaffs_mkdir(fn,0);
		}

		for(i = 0; i < 10; i++)
		{
		  sprintf(fn,"%s/%d",p,i);
		  if(i & 1)
		  	yaffs_unlink(fn);
		  else
		  	yaffs_rmdir(fn);

		}

		yaffs_unmount(mountpt);
	}




}


void write_10k(int h)
{
   int i;
   const char *s="0123456789";
   for(i = 0; i < 1000; i++)
     yaffs_write(h,s,10);

}
void write_200k_file(const char *fn, const char *fdel, const char *fdel1)
{
   int h1;
   int i;
   int offs;

   h1 = yaffs_open(fn, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

   for(i = 0; i < 100000; i+= 10000)
   {
   	write_10k(h1);
   }

   offs = yaffs_lseek(h1,0,SEEK_CUR);
   if( offs != 100000)
   {
   	printf("Could not write file\n");
   }

   yaffs_unlink(fdel);
   for(i = 0; i < 100000; i+= 10000)
   {
   	write_10k(h1);
   }

   offs = yaffs_lseek(h1,0,SEEK_CUR);
   if( offs != 200000)
   {
   	printf("Could not write file\n");
   }

   yaffs_close(h1);
   yaffs_unlink(fdel1);

}


void verify_200k_file(const char *fn)
{
   int h1;
   int i;
   char x[11];
   const char *s="0123456789";
   int errCount = 0;

   h1 = yaffs_open(fn, O_RDONLY, 0);

   for(i = 0; i < 200000 && errCount < 10; i+= 10)
   {
   	yaffs_read(h1,x,10);
	if(strncmp(x,s,10) != 0)
	{
		printf("File %s verification failed at %d\n",fn,i);
		errCount++;
	}
   }
   if(errCount >= 10)
   	printf("Too many errors... aborted\n");

   yaffs_close(h1);

}


void check_resize_gc_bug(const char *mountpt)
{

	char a[30];
	char b[30];
	char c[30];

	int i;

	sprintf(a,"%s/a",mountpt);
	sprintf(b,"%s/b",mountpt);
	sprintf(c,"%s/c",mountpt);




	yaffs_start_up();
	yaffs_mount(mountpt);
	yaffs_unlink(a);
	yaffs_unlink(b);

	for(i = 0; i < 50; i++)
	{
	   printf("A\n");write_200k_file(a,"",c);
	   printf("B\n");verify_200k_file(a);
	   printf("C\n");write_200k_file(b,a,c);
	   printf("D\n");verify_200k_file(b);
	   yaffs_unmount(mountpt);
	   yaffs_mount(mountpt);
	   printf("E\n");verify_200k_file(a);
	   printf("F\n");verify_200k_file(b);
	}

}


void multi_mount_test(const char *mountpt,int nmounts)
{

	char a[30];

	int i;
	int j;

	sprintf(a,"%s/a",mountpt);

	yaffs_start_up();

	for(i = 0; i < nmounts; i++){
		int h0;
		int h1;
		int len0;
		int len1;

		static char xx[1000];

		printf("############### Iteration %d   Start\n",i);
		if(1 || i == 0 || i == 5)
			yaffs_mount(mountpt);

		dump_directory_tree(mountpt);


		yaffs_mkdir(a,0);

		sprintf(xx,"%s/0",a);
		h0 = yaffs_open(xx, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

		sprintf(xx,"%s/1",a);
		h1 = yaffs_open(xx, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

#if 0
		for(j = 0; j < 200; j++){
		   yaffs_write(h0,xx,1000);
		   yaffs_write(h1,xx,1000);
		}
#else
		while(yaffs_write(h0,xx,1000) > 0){

		   yaffs_write(h1,xx,1000);
		}
#endif
		len0 = yaffs_lseek(h0,0,SEEK_END);
		len1 = yaffs_lseek(h1,0,SEEK_END);

		yaffs_lseek(h0,0,SEEK_SET);
		yaffs_lseek(h1,0,SEEK_SET);

		for(j = 0; j < 200; j++){
		   yaffs_read(h0,xx,1000);
		   yaffs_read(h1,xx,1000);
		}


	//	yaffs_truncate(h0,0);
		yaffs_close(h0);
		yaffs_close(h1);

		printf("########### %d\n",i);
		dump_directory_tree(mountpt);

		if(1 || i == 4 || i == nmounts -1)
			yaffs_unmount(mountpt);
	}
}


void small_mount_test(const char *mountpt,int nmounts)
{

	char a[30];

	int i;
	int j;

	int h0;
	int h1;
	int len0;
	int len1;
	int nread;

	sprintf(a,"%s/a",mountpt);

	yaffs_start_up();



	for(i = 0; i < nmounts; i++){

		static char xx[1000];

		printf("############### Iteration %d   Start\n",i);
		if(1 || i == 0 || i == 5)
			yaffs_mount(mountpt);

		dump_directory_tree(mountpt);

		yaffs_mkdir(a,0);

		sprintf(xx,"%s/0",a);
		if(i ==0){

			h0 = yaffs_open(xx, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
			for(j = 0; j < 130; j++)
				yaffs_write(h0,xx,1000);
			yaffs_close(h0);
		}

		h0 = yaffs_open(xx,O_RDONLY,0);

		sprintf(xx,"%s/1",a);
		h1 = yaffs_open(xx, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

		while((nread = yaffs_read(h0,xx,1000)) > 0)
			yaffs_write(h1,xx,nread);


		len0 = yaffs_lseek(h0,0,SEEK_END);
		len1 = yaffs_lseek(h1,0,SEEK_END);

		yaffs_lseek(h0,0,SEEK_SET);
		yaffs_lseek(h1,0,SEEK_SET);

		for(j = 0; j < 200; j++){
		   yaffs_read(h0,xx,1000);
		   yaffs_read(h1,xx,1000);
		}

		yaffs_close(h0);
		yaffs_close(h1);

		printf("########### %d\n",i);
		dump_directory_tree(mountpt);

		if(1 || i == 4 || i == nmounts -1)
			yaffs_unmount(mountpt);
	}
}


int early_exit;

void small_overwrite_test(const char *mountpt,int nmounts)
{

	char a[30];
	int i;
	int j;

	int h0;
	int h1;


	sprintf(a,"%s/a",mountpt);

	yaffs_start_up();



	for(i = 0; i < nmounts; i++){

		static char xx[8000];

		printf("############### Iteration %d   Start\n",i);
		if(1)
			yaffs_mount(mountpt);

		dump_directory_tree(mountpt);

		yaffs_mkdir(a,0);

		sprintf(xx,"%s/0",a);
		h0 = yaffs_open(xx, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
		sprintf(xx,"%s/1",a);
		h1 = yaffs_open(xx, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

		for(j = 0; j < 1000000; j+=1000){
			yaffs_ftruncate(h0,j);
			yaffs_lseek(h0,j,SEEK_SET);
			yaffs_write(h0,xx,7000);
			yaffs_write(h1,xx,7000);

			if(early_exit)
				exit(0);
		}

		yaffs_close(h0);

		printf("########### %d\n",i);
		dump_directory_tree(mountpt);

		if(1)
			yaffs_unmount(mountpt);
	}
}


void seek_overwrite_test(const char *mountpt,int nmounts)
{
        static char xx[5000];
	char a[30];

	int i;
	int j;

	int h0;


	sprintf(a,"%s/f",mountpt);

	yaffs_start_up();

	yaffs_mount(mountpt);


	for(i = 0; i < nmounts; i++){

		h0 = yaffs_open(a, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);

		for(j = 0; j < 100000; j++){
			yaffs_lseek(h0,0,SEEK_SET);
			yaffs_write(h0,xx,5000);
			yaffs_lseek(h0,0x100000,SEEK_SET);
			yaffs_write(h0,xx,5000);

			if(early_exit)
				exit(0);
		}

		yaffs_close(h0);

	}
}


void yaffs_touch(const char *fn)
{
	yaffs_chmod(fn, S_IREAD | S_IWRITE);
}

void checkpoint_fill_test(const char *mountpt,int nmounts)
{

	char a[50];
	char b[50];
	char c[50];

	int i;
	int j;
	int h;

	sprintf(a,"%s/a",mountpt);




	yaffs_start_up();

	for(i = 0; i < nmounts; i++){
		printf("############### Iteration %d   Start\n",i);
		yaffs_mount(mountpt);
		dump_directory_tree(mountpt);
		yaffs_mkdir(a,0);

		sprintf(b,"%s/zz",a);

		h = yaffs_open(b,O_CREAT | O_RDWR,S_IREAD |S_IWRITE);


		while(yaffs_write(h,c,50) == 50){}

		yaffs_close(h);

		for(j = 0; j < 2; j++){
			printf("touch %d\n",j);
			yaffs_touch(b);
			yaffs_unmount(mountpt);
			yaffs_mount(mountpt);
		}

		dump_directory_tree(mountpt);
		yaffs_unmount(mountpt);
	}
}


int make_file2(const char *name1, const char *name2,int syz)
{

	char xx[2500];
	int i;
	int h1=-1,h2=-1;
	int n = 1;


	if(name1)
		h1 = yaffs_open(name1,O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
	if(name2)
		h2 = yaffs_open(name2,O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

	while(syz > 0 && n > 0){
		i = (syz > 2500) ? 2500 : syz;
		n = yaffs_write(h1,xx,i);
		n = yaffs_write(h2,xx,i);
		syz -= 500;
	}
	yaffs_close(h1);
	yaffs_close(h2);
	return 0;
}


extern void SetCheckpointReservedBlocks(int n);

void checkpoint_upgrade_test(const char *mountpt,int nmounts)
{

	char a[50];
	char b[50];
	char c[50];
	char d[50];

	int j;

	sprintf(a,"%s/a",mountpt);




	printf("Create start condition\n");
	yaffs_start_up();
	yaffs_mount(mountpt);
	yaffs_mkdir(a,0);
	sprintf(b,"%s/zz",a);
	sprintf(c,"%s/xx",a);
	make_file2(b,c,2000000);
	sprintf(d,"%s/aa",a);
	make_file2(d,NULL,500000000);
	dump_directory_tree(mountpt);

	printf("Umount/mount attempt full\n");
	yaffs_unmount(mountpt);

	yaffs_mount(mountpt);

	printf("unlink small file\n");
	yaffs_unlink(c);
	dump_directory_tree(mountpt);

	printf("Umount/mount attempt\n");
	yaffs_unmount(mountpt);
	yaffs_mount(mountpt);

	for(j = 0; j < 500; j++){
		printf("***** touch %d\n",j);
		dump_directory_tree(mountpt);
		yaffs_touch(b);
		yaffs_unmount(mountpt);
		yaffs_mount(mountpt);
	}

	for(j = 0; j < 500; j++){
		printf("***** touch %d\n",j);
		dump_directory_tree(mountpt);
		yaffs_touch(b);
		yaffs_unmount(mountpt);
		yaffs_mount(mountpt);
	}
}

void huge_array_test(const char *mountpt,int n)
{

	char a[50];


	int i;
	int space;

	int fnum;

	sprintf(a,"mount point %s",mountpt);



	yaffs_start_up();

	yaffs_mount(mountpt);

	while(n>0){
		n--;
		fnum = 0;
		printf("\n\n START run\n\n");
		while((space = yaffs_freespace(mountpt)) > 25000000){
			sprintf(a,"%s/file%d",mountpt,fnum);
			fnum++;
			printf("create file %s, free space %d\n",a,space);
			create_file_of_size(a,10000000);
			printf("verifying file %s\n",a);
			verify_file_of_size(a,10000000);
		}

		printf("\n\n verification/deletion\n\n");

		for(i = 0; i < fnum; i++){
			sprintf(a,"%s/file%d",mountpt,i);
			printf("verifying file %s\n",a);
			verify_file_of_size(a,10000000);
			printf("deleting file %s\n",a);
			yaffs_unlink(a);
		}
		printf("\n\n done \n\n");


	}
}


void random_write(int h)
{
	static char buffer[12000];
	int n;

	n = random() & 0x1FFF;
	yaffs_write(h,buffer,n);
}

void random_seek(int h)
{
	int n;
	n = random() & 0xFFFFF;
	yaffs_lseek(h,n,SEEK_SET);
}

void random_truncate(int h, char * name)
{
	int n;
	int flen;
	n = random() & 0xFFFFF;
	flen = yaffs_lseek(h,0,SEEK_END);
	if(n > flen)
		n = flen / 2;
	yaffs_ftruncate(h,n);
	yaffs_lseek(h,n,SEEK_SET);
}


#define NSMALLFILES 10
void random_small_file_test(const char *mountpt,int iterations)
{

	char a[NSMALLFILES][50];


	int i;
	int n;
	int h[NSMALLFILES];
	int r;


	yaffs_start_up();

	yaffs_mount(mountpt);

	for(i = 0; i < NSMALLFILES; i++){
		h[i]=-1;
		strcpy(a[i],"");
	}

	for(n = 0; n < iterations; n++){

		for(i = 0; i < NSMALLFILES; i++) {
			r = random();

			if(strlen(a[i]) == 0){
				sprintf(a[i],"%s/%dx%d",mountpt,n,i);
				h[i] = yaffs_open(a[i],O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
			}

			if(h[i] < -1)
				printf("Could not open yaffs file %d %d error %d\n",n,i,h[i]);
			else {
				r = r & 7;
				switch(r){
					case 0:
					case 1:
					case 2:
						random_write(h[i]);
						break;
					case 3:
						random_truncate(h[i],a[i]);
						break;
					case 4:
					case 5:	random_seek(h[i]);
						break;
					case 6:
						yaffs_close(h[i]);
						h[i] = -1;
						break;
					case 7:
						yaffs_close(h[i]);
						yaffs_unlink(a[i]);
						strcpy(a[i],"");
						h[i] = -1;
				}
			}
		}

	}

	for(i = 0; i < NSMALLFILES; i++)
		yaffs_close(h[i]);

	yaffs_unmount(mountpt);
}

void rmdir_test(const char *mountpt)
{
	char name[100];
	yaffs_start_up();

	yaffs_mount(mountpt);

	strcpy(name,mountpt);
	strcat(name,"/");
	strcat(name,"hello");
	yaffs_mkdir(name,0666);
	yaffs_rmdir(name);
	yaffs_unmount(mountpt);
}



static void print_xattrib_val(const char *path, const char *name)
{
	char buffer[1000];
	int n;

	n = yaffs_getxattr(path,name,buffer,sizeof(buffer));
	if(n >= 0){
		u8 *b = (u8 *)buffer;

		printf("%d bytes:",n);
		if(n > 10)
			n = 10;
		while(n > 0){
			printf("[%02X]",*b);
			b++;
			n--;
		}
		printf("\n");
	} else
		printf(" Novalue result %d\n",n);
}

static void list_xattr(const char *path)
{
	char list[1000];
	int n=0;
	int list_len;
	int len;

	list_len = yaffs_listxattr(path,list,sizeof(list));
	printf("xattribs for %s, result is %d\n",path,list_len);
	while(n < list_len){
		len = strlen(list + n);
		printf("\"%s\" value ",list+n);
		print_xattrib_val(path,list + n);
		n += (len + 1);
	}
	printf("end\n");
}

void basic_utime_test(const char *mountpt)
{
	char name[100];
	int h;
	int result;
	int val1;
	struct yaffs_utimbuf utb;
	struct yaffs_stat st;

	yaffs_start_up();

	yaffs_mount(mountpt);

	strcpy(name,mountpt);
	strcat(name,"/");
	strcat(name,"xfile");

	yaffs_unlink(name);

	printf("created\n");
	h = yaffs_open(name,O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

	yaffs_fstat(h,&st); printf(" times %u %u %u\n",st.yst_atime, st.yst_ctime, st.yst_mtime);

	utb.actime = 1000;
	utb.modtime = 2000;
	result = yaffs_futime(h,&utb);
	printf("futime to a 1000 m 2000 result %d\n",result);
	yaffs_fstat(h,&st); printf(" times %u %u %u\n",st.yst_atime, st.yst_ctime, st.yst_mtime);


	utb.actime = 5000;
	utb.modtime = 8000;
	result = yaffs_utime(name, &utb);
	printf("utime to a 5000 m 8000 result %d\n",result);
	yaffs_fstat(h,&st); printf(" times %u %u %u\n",st.yst_atime, st.yst_ctime, st.yst_mtime);

	result = yaffs_utime(name, NULL);
	printf("utime to NULL result %d\n",result);
	yaffs_fstat(h,&st); printf(" times %u %u %u\n",st.yst_atime, st.yst_ctime, st.yst_mtime);


}

void basic_xattr_test(const char *mountpt)
{
	char name[100];
	int h;
	int result;
	int val1;

	yaffs_start_up();

	yaffs_mount(mountpt);

	strcpy(name,mountpt);
	strcat(name,"/");
	strcat(name,"xfile");

	yaffs_unlink(name);
	h = yaffs_open(name,O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
	yaffs_close(h);

	printf("Start\n");
	list_xattr(name);

	printf("Add an attribute\n");
	val1 = 0x123456;
	result = yaffs_setxattr(name,"foo",&val1,sizeof(val1),0);
	printf("wrote attribute foo: result %d\n",result);
	list_xattr(name);
	printf("Add an attribute\n");
	val1 = 0x7890;
	result = yaffs_setxattr(name,"bar",&val1,sizeof(val1),0);
	printf("wrote attribute bar: result %d\n",result);
	list_xattr(name);

	printf("Get non-existanrt attribute\n");
	print_xattrib_val(name,"not here");

	printf("Delete non existing attribute\n");
	yaffs_removexattr(name,"not here");
	list_xattr(name);

	printf("Remove foo\n");
	yaffs_removexattr(name,"foo");
	list_xattr(name);

	printf("Remove bar\n");
	yaffs_removexattr(name,"bar");
	list_xattr(name);

}

void big_xattr_test(const char *mountpt)
{
	char name[100];
	int h;
	int result;
	char val[1000];

	yaffs_start_up();

	yaffs_mount(mountpt);

	strcpy(name,mountpt);
	strcat(name,"/");
	strcat(name,"xfile");

	yaffs_unlink(name);
	h = yaffs_open(name,O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
	yaffs_close(h);

	printf("Start\n");
	list_xattr(name);

	printf("Add a large  attribute\n");
	memset(val,0x1,sizeof(val));
	result = yaffs_setxattr(name,"aaa",val,200,0);
	printf("wrote attribute aaa: result %d\n",result);
	list_xattr(name);

	printf("Add a large  attribute\n");
	memset(val,0x2,sizeof(val));
	result = yaffs_setxattr(name,"bbb",val,1000,0);
	printf("wrote attribute bbb: result %d\n",result);
	list_xattr(name);

	printf("Replace attribute\n");
	memset(val,0x3,sizeof(val));
	result = yaffs_setxattr(name,"aaa",val,1000,0);
	printf("wrote attribute aaa: result %d\n",result);
	list_xattr(name);

}


void dump_dev_stats(struct yaffs_dev *dev, const char * str)
{
	printf("%s\n",str);
	printf( "space free %d erased %d "
		"nand reads %d writes %d erases %d "
		"gc all %d passive %d oldestdirty %d blocks %d copies %d \n",
		dev->n_free_chunks, dev->n_erased_blocks * dev->param.chunks_per_block,
		dev->n_page_reads, dev->n_page_writes, dev->n_erasures,
		dev->all_gcs, dev->passive_gc_count, dev->oldest_dirty_gc_count, dev->n_gc_blocks, dev->n_gc_copies);
}

void test_flash_traffic(const char *mountpt)
{
	char name0[100];
	char name1[100];
	int i;
	struct yaffs_dev *dev;

	yaffs_trace_mask = 0;

	yaffs_start_up();

	yaffs_mount(mountpt);

	dev = yaffs_getdev(mountpt);

	strcpy(name0,mountpt);
	strcat(name0,"/x");

	strcpy(name1,mountpt);
	strcat(name1,"/y");

	dump_dev_stats(dev,"start");
	create_file_of_size(name0,32 * 1024 * 1024);
	dump_dev_stats(dev,"32MB written");
	for(i = 0; i < 20; i++)
		create_file_of_size(name1,1024 * 1024);
	dump_dev_stats(dev,"20x 1MB files written");

}

void link_follow_test(const char *mountpt)
{
	char fn[100];
	char sn[100];
	char hn[100];
	int result;
	int h;

	yaffs_trace_mask = 0;

	yaffs_start_up();

	yaffs_mount(mountpt);

	sprintf(fn,"%s/file",mountpt);
	sprintf(sn,"%s/sym",mountpt);
	sprintf(hn,"%s/hl-sym",mountpt);

	h = yaffs_open(fn,O_CREAT| O_RDWR, S_IREAD | S_IWRITE);
	result = yaffs_close(h);

	result = yaffs_symlink(fn,sn);
	result = yaffs_link(sn,hn);

	h =yaffs_open(hn,O_RDWR,0);

}

void max_files_test(const char *mountpt)
{
	char fn[100];
	char sn[100];
	char hn[100];
	int result;
	int h;
	int i;

	yaffs_trace_mask = 0;

	yaffs_start_up();

	yaffs_mount(mountpt);

	for(i = 0; i < 5000; i++) {
		sprintf(fn,"%s/file%d", mountpt, i);
		yaffs_unlink(fn);
		h = yaffs_open(fn,O_CREAT| O_RDWR, S_IREAD | S_IWRITE);
		if(h < 0)
			printf("File %s not created\n", fn);
		yaffs_write(h,fn,100);
		result = yaffs_close(h);
	}
	for(i = 0; i < 5; i++){
		sprintf(fn,"%s/file%d",mountpt, i);
		yaffs_unlink(fn);
	}

	for(i = 1000; i < 1010; i++){
		sprintf(fn,"%s/file%d",mountpt, i);
		h = yaffs_open(fn,O_CREAT| O_RDWR, S_IREAD | S_IWRITE);
		yaffs_write(h,fn,100);
		if(h < 0)
			printf("File %s not created\n", fn);
		result = yaffs_close(h);
	}

	yaffs_unmount(mountpt);

	//h =yaffs_open(hn,O_RDWR,0);

}
void case_insensitive_test(const char *mountpt)
{
        char fn[100];
        char fn2[100];
        char buffer[100];
        int ret;
        struct yaffs_stat s;
        int h;
        char *x;

	yaffs_trace_mask = 0;

	yaffs_start_up();

	yaffs_mount(mountpt);
	dump_directory_tree(mountpt);

	sprintf(fn,"%s/Abc.Txt",mountpt);
	yaffs_unlink(fn);
	h = yaffs_open(fn, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);

	ret = yaffs_write(h,fn, strlen(fn) + 1);

	ret = yaffs_close(h);

	dump_directory_tree(mountpt);


	strcpy(fn2, fn);
	x = fn2;
	while(*x) {
		*x = toupper(*x);
		x++;
        }

        h = yaffs_open(fn2, O_RDONLY, 0);
        ret = yaffs_read(h, buffer, 100);

        if (ret != strlen(fn) + 1 || memcmp(buffer, fn, ret)){
		printf("wrong file read\n");
        } else {
		printf("File %s is the same as file %s\n", fn, fn2);
        }

        ret = yaffs_stat(fn2, &s);

	printf("renaming\n");

        ret = yaffs_rename(fn, fn2);
 	dump_directory_tree(mountpt);

}

void start_twice(const char *mountpt)
{
         printf("About to do first yaffs_start\n");
         yaffs_start_up();
         printf("started\n");
         printf("First mount returns %d\n", yaffs_mount(mountpt));
         printf("About to do second yaffs_start\n");
         yaffs_start_up();
         printf("started\n");
         printf("Second mount returns %d\n", yaffs_mount(mountpt));
}

#define N_WRITES 2000
#define STRIDE	 2000

#define BUFFER_N 1100
unsigned  xxbuffer[BUFFER_N];


void set_buffer(int n)
{
	int i;
	for(i = 0; i < BUFFER_N; i++)
		xxbuffer[i] = i + n;
}

void write_big_sparse_file(int h)
{
	int i;
	loff_t offset = 0;
	loff_t pos;
	int n = sizeof(xxbuffer);
	int wrote;

	for(i = 0; i < N_WRITES; i++) {
		printf("writing at %lld\n", offset);
		set_buffer(i);
		pos = yaffs_lseek(h, offset, SEEK_SET);
		if(pos != offset) {
			printf("mismatched seek pos %lld offset %lld\n",
				pos, offset);
			perror("lseek64");
			exit(1);
		}
		wrote = yaffs_write(h, xxbuffer, n);

		if(wrote != n) {
			printf("mismatched write wrote %d n %d\n", wrote, n);
			exit(1);
		}

		offset += (STRIDE * sizeof(xxbuffer));
	}

	yaffs_ftruncate(h, offset);

}




void verify_big_sparse_file(int h)
{
	unsigned check_buffer[BUFFER_N];
	int i;
	loff_t offset = 0;
	loff_t pos;
	int n = sizeof(check_buffer);
	int result;
	const char * check_type;
	int checks_failed = 0;
	int checks_passed = 0;

	for(i = 0; i < N_WRITES * STRIDE; i++) {
		if(i % STRIDE) {
			check_type = "zero";
			memset(xxbuffer,0, n);
		} else {
			check_type = "buffer";
			set_buffer(i/STRIDE);
		}
		//printf("%s checking %lld\n", check_type, offset);
		pos = yaffs_lseek(h, offset, SEEK_SET);
		if(pos != offset) {
			printf("mismatched seek pos %lld offset %lld\n",
				pos, offset);
			perror("lseek64");
			exit(1);
		}
		result = yaffs_read(h, check_buffer, n);

		if(result != n) {
			printf("mismatched read result %d n %d\n", result, n);
			exit(1);
		}




		if(memcmp(xxbuffer, check_buffer, n)) {
			int j;

			printf("buffer at %lld mismatches\n", pos);
			printf("xxbuffer ");
			for(j = 0; j < 20; j++)
				printf(" %d",xxbuffer[j]);
			printf("\n");
			printf("check_buffer ");
			for(j = 0; j < 20; j++)
				printf(" %d",check_buffer[j]);
			printf("\n");

			checks_failed++;
		} else {
			checks_passed++;
		}

		offset += sizeof(xxbuffer);
	}

	printf("%d checks passed, %d checks failed\n", checks_passed, checks_failed);

}


void large_file_test(const char *mountpt)
{
	char xx_buffer[1000];
	int i;
	int handle;
	char fullname[100];
	loff_t file_end;

	yaffs_trace_mask = 0;

	yaffs_start_up();

	yaffs_mount(mountpt);
	printf("mounted\n");
        dumpDir(mountpt);

	sprintf(fullname, "%s/%s", mountpt, "big-test-file");

	handle = yaffs_open(fullname, O_RDONLY, 0);

	handle = yaffs_open(fullname, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

	if(handle < 0) {
		perror("opening file");
		exit(1);
	}

	write_big_sparse_file(handle);
	verify_big_sparse_file(handle);

	yaffs_close(handle);

	printf("Job done\n");
	yaffs_unmount(mountpt);

	yaffs_mount(mountpt);
	printf("mounted again\n");
        dumpDir(mountpt);
	handle = yaffs_open(fullname, O_RDONLY, 0);
	verify_big_sparse_file(handle);
	yaffs_unmount(mountpt);


	yaffs_mount_common(mountpt, 0, 1);
	printf("mounted with no checkpt\n");
        dumpDir(mountpt);
	handle = yaffs_open(fullname, O_RDONLY, 0);
	verify_big_sparse_file(handle);
	yaffs_unmount(mountpt);

	/* Check resize by adding to the end, resizing back and verifying. */
	yaffs_mount_common(mountpt, 0, 1);
	printf("checking resize\n");
        dumpDir(mountpt);
	handle = yaffs_open(fullname, O_RDWR, 0);

	file_end = yaffs_lseek(handle, 0, SEEK_END);
	printf("file_end %lld\n", file_end);
	for(i = 0; i < 10000; i++)
		yaffs_write(handle, xx_buffer, sizeof(xx_buffer));
	yaffs_ftruncate(handle, file_end);

	verify_big_sparse_file(handle);
	yaffs_unmount(mountpt);

}


int  mk_dir(const char *mp, const char *name)
{
	char full_name[100];

	sprintf(full_name, "%s/%s", mp, name);

	return yaffs_mkdir(full_name, S_IREAD| S_IWRITE);
}

int  mk_file(const char *mp, const char *name)
{
	char full_name[100];
	int h;

	sprintf(full_name, "%s/%s", mp, name);

	h = yaffs_open(full_name, O_RDWR | O_CREAT | O_TRUNC, S_IREAD| S_IWRITE);

	yaffs_write(h, name, strlen(name));

	yaffs_close(h);
	return 0;
}

void xx_test(const char *mountpt)
{
	char xx_buffer[1000];

	yaffs_start_up();

	yaffs_format(mountpt,0,0,0);

	yaffs_mount(mountpt);
	printf("mounted\n");
	dumpDir(mountpt);

	printf("create files\n");

	mk_dir(mountpt, "foo");
	mk_file(mountpt, "foo/f1");
	mk_file(mountpt, "foo/f2");
	mk_file(mountpt, "foo/f3");
	mk_file(mountpt, "foo/f4");
	dump_directory_tree(mountpt);

	printf("unmount and remount\n");

	/* Unmount/remount */
	yaffs_unmount(mountpt);
	yaffs_mount(mountpt);
	dump_directory_tree(mountpt);
}

void yy_test(const char *mountpt)
{
	char xx_buffer[1000];

	yaffs_start_up();

	yaffs_mount(mountpt);
	dump_directory_tree(mountpt);
}


void readdir_test(const char *mountpt)
{
	char xx_buffer[1000];
	int i;
	int handle;
	char fullname[100];

	yaffs_DIR *dirs[100];


	yaffs_trace_mask = 0;

	yaffs_start_up();

	yaffs_mount(mountpt);

	for(i = 0; i < 100; i++) {
	         dirs[i] = yaffs_opendir(mountpt);
	         printf("%2d %p,", i, dirs[i]);
	}

	printf("\n");

	for(i = 0; i < 100; i++) {
	         if(dirs[i])
	                  yaffs_closedir(dirs[i]);
	}


	for(i = 0; i < 100; i++) {
	         dirs[i] = yaffs_opendir(mountpt);
	         printf("%2d %p,", i, dirs[i]);
	}

	yaffs_unmount(mountpt);


}

void format_test(const char *mountpt)
{
	int ret;

	yaffs_start_up();

	ret = yaffs_format(mountpt, 0, 0, 0);
	printf("yaffs_format(...,0, 0, 0) of unmounted returned %d\n", ret);

	yaffs_mount(mountpt);

	ret = yaffs_format(mountpt, 0, 0, 0);
	printf("yaffs_format(...,0, 0, 0) of mounted returned %d\n", ret);

	ret = yaffs_format(mountpt, 1, 0, 0);
	printf("yaffs_format(...,1, 0, 0) of mounted returned %d\n", ret);

	ret = yaffs_mount(mountpt);
	printf("mount should return 0 returned %d\n", ret);

	ret = yaffs_format(mountpt, 1, 0, 1);
	printf("yaffs_format(...,1, 0, 1) of mounted returned %d\n", ret);

	ret = yaffs_mount(mountpt);
	printf("mount should return -1 returned %d\n", ret);
}

void dir_rename_test(const char *mountpt)
{
         char fname[100];
         char dname[100];
         int h;
         int ret;

         yaffs_start_up();
         yaffs_mount(mountpt);

         sprintf(fname,"%s/file",mountpt);
         sprintf(dname,"%s/directory",mountpt);

         h = yaffs_open(fname,O_CREAT | O_RDWR | O_TRUNC, 0666);
         yaffs_close(h);

         yaffs_mkdir(dname, 0666);

         dump_directory_tree(mountpt);

         printf("Try to rename %s to %s\n", fname, dname);
         ret = yaffs_rename(fname, dname);
         printf("result %d, %d\n", ret, yaffs_get_error());

         printf("Try to rename %s to %s\n", dname, fname);
         ret = yaffs_rename(dname, fname);
         printf("result %d, %d\n", ret, yaffs_get_error());


}

int random_seed;
int simulate_power_failure;

int main(int argc, char *argv[])
{
	random_seed = time(NULL);
	//return long_test(argc,argv);

	//return cache_read_test();

	// resize_stress_test_no_grow("/flash/flash",20);
	//root_perm_remount("/flash/flash");

	//huge_directory_test_on_path("/ram2k");

	 //yaffs_backward_scan_test("/flash/flash");
	// yaffs_device_flush_test("/flash/flash");

	//rename_over_test("//////////////////flash///////////////////yaffs1///////////");

	//fill_empty_files_test("/yaffs2/");
	//resize_stress_test("/yaffs2");
	//overwrite_test("/yaffs2");

	//long_name_test("/yaffs2");
	//link_test0("/yaffs2");
	//link_test1("yaffs2");
	 //scan_pattern_test("/flash",10000,10);
	//short_scan_test("/flash/flash",40000,200);
	  //small_mount_test("/flash/flash",1000);
	  //small_overwrite_test("/flash/flash",1000);
	  //seek_overwrite_test("/flash/flash",1000);
	 //checkpoint_fill_test("/flash/flash",20);
	 //checkpoint_upgrade_test("/flash/flash",20);
	  //small_overwrite_test("/flash/flash",1000);
	  //checkpoint_fill_test("/flash/flash",20);
	//random_small_file_test("/flash/flash",10000);
	 // huge_array_test("/flash/flash",10);


	// handle_test("yaffs2/");

	//long_test_on_path("/ram2k");
	// long_test_on_path("/flash");
	//simple_rw_test("/flash/flash");
	//fill_disk_test("/flash/flash");
	// rename_over_test("/flash");
	//lookup_test("/flash");
	//freespace_test("/flash/flash");

	//link_test("/flash/flash");

	// cache_bypass_bug_test();

	 //free_space_check();

	 //check_resize_gc_bug("/flash");

	 //basic_xattr_test("/yaffs2");
	 //big_xattr_test("/yaffs2");

	 //null_name_test("yaffs2");

	 //test_flash_traffic("yaffs2");
	 // link_follow_test("/yaffs2");
	 //basic_utime_test("/yaffs2");


	//format_test("/yaffs2");

	//max_files_test("/yaffs2");

	 //start_twice("/yaffs2");

	 //large_file_test("/yaffs2");
	 //readdir_test("/yaffs2");

	 //basic_utime_test("/yaffs2");
	 //case_insensitive_test("/yaffs2");

	 //yy_test("/yaffs2");
	 dir_rename_test("/yaffs2");

	 return 0;

}
