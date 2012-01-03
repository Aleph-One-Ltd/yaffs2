#define _LARGEFILE64_SOURCE
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#define N_WRITES 16
#define STRIDE	 250000

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
		pos = lseek64(h, offset, SEEK_SET);
		if(pos != offset) {
			printf("mismatched seek pos %lld offset %lld\n",
				pos, offset);
			perror("lseek64");
			exit(1);
		}
		wrote = write(h, xxbuffer, n);

		if(wrote != n) {
			printf("mismatched write wrote %d n %d\n", wrote, n);
			exit(1);
		}

		offset += (STRIDE * sizeof(xxbuffer));
	}

	ftruncate(h, offset);

}


int main(int argc, char *argv[])
{
	int handle;

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

	printf("Job done\n");
	return 0;
}
