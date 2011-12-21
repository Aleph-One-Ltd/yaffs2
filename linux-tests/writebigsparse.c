#define _LARGEFILE64_SOURCE
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int handle;

#define N_WRITES 8000
#define STRIDE	 1000

#define BUFFER_N 2000
unsigned  buffer[BUFFER_N];


void set_buffer(int n)
{
	int i;
	for(i = 0; i < BUFFER_N; i++)
		buffer[i] = i + n;
}

void write_big_sparse_file(int h)
{
	int i;
	off64_t offset = 0;
	off64_t pos;
	int n = sizeof(buffer);
	int wrote;
	
	for(i = 0; i < 4000; i++) {
		set_buffer(i);
		pos = lseek64(h, offset, SEEK_SET);
		if(pos != offset) {
			printf("mismatched seek pos %lld offset %lld\n",
				pos, offset);
			perror("lseek64");
			exit(1);
		}
		wrote = write(h, buffer, n);
		
		if(wrote != n) {
			printf("mismatched write wrote %d n %d\n", wrote, n);
			exit(1);
		}
		
		offset += (STRIDE * sizeof(buffer));
	}
}




void verify_big_sparse_file(int h)
{
}

int main(int argc, char *argv[])
{

	if(argc < 2) {
		printf("Gimme a file name!\n");
		exit(1);
	}
	
	handle = open(argv[1], O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	
	if(handle < 0) {
		perror("opening file");
		exit(1);
	}
	
	write_big_sparse_file(handle);
	system("sudo echo 3 > /proc/sys/vm/drop_caches");
	verify_big_sparse_file(handle);
	
	printf("Job done\n");
	return 0;
}
