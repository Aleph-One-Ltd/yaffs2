#include <stdio.h>
#include <errno.h>


int main()
{
	int output=0;
	int error_code =0 ;
	output=rename( "/home/timothy/test/cat/","/home/timothy/test/dog/");
	printf("output %d\n",output);
	if (output <0 ) {
		error_code = errno;
		printf("error code %d\n",error_code);
	        printf("Error description is : %s\n",strerror(errno));
	}
	return 0;
}
