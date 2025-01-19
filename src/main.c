#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <script-tree.h>

#define VERSION "1.1.0"

extern void lunch(int* argc,char* argv[]);

static const Command commandList[] = {
	{"lunch",lunch,"lunch work space."}
};

int main(int argc,char* argv[])
{
	//print version when no input 
	if(argc < 2){
		printf("%s version %s\n",argv[0],VERSION);
		exit(0);
	}
	
	//check comand input to call command function
	int i;
	int commandCount = sizeof(commandList)/sizeof(Command);
	for(i = 0;i < commandCount;i++){
		if(strcmp(argv[1],commandList[i].name) == 0){
			argc -= 2;
			argv += 2;
			commandList[i].func(&argc,argv);
			exit(0);
		}
	}
	
	fprintf(stderr,"%s is invalid arg\n",argv[1]);

	return 0;
}

