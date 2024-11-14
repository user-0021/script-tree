#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define NAME "programNodeSystem"

static const char fifo_in_path[] = "/tmp/"NAME"_in";
static const char fifo_out_path[] = "/tmp/"NAME"_out";

void lunch(int* argc,char* argv[]);

int main(int argc,char* argv[])
{
	lunch(&argc,argv);
	return 0;
}

void lunch(int* argc,char* argv[]){
	int in,out;

	//make fifo
	if(mkfifo(fifo_in_path,0666)){
		perror("open pipe in");
	}
	if(mkfifo(fifo_out_path,0666)){
		perror("open pipe out");
	}
	
	printf("A\n");
	if((in=open(fifo_in_path,O_WRONLY | O_NONBLOCK))==-1){
		perror("open");
		exit(-1);
	}

	printf("A\n");
	if((out=open(fifo_out_path,O_RDONLY | O_NONBLOCK))==-1){
		perror("open");
		exit(-1);
	}

	
	while(getchar() != 'a'){
	}

	printf("B\n");
	close(in);
	printf("C\n");
	close(out);
	printf("D\n");
}
