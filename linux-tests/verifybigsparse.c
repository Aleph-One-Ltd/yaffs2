#define _LARGEFILE64_SOURCE
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


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
		pos = lseek64(h, offset, SEEK_SET);
		if(pos != offset) {
			printf("mismatched seek pos %lld offset %lld\n",
				pos, offset);
			perror("lseek64");
			exit(1);
		}
		result = read(h, check_buffer, n);

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


int main(int argc, char *argv[])
{
	int handle;

	if(argc < 2) {
		printf("Gimme a file name!\n");
		exit(1);
	}

	handle = open(argv[1], O_RDONLY);

	if(handle < 0) {
		perror("opening file");
		exit(1);
	}

	verify_big_sparse_file(handle);

	printf("Job done\n");
	return 0;
}
