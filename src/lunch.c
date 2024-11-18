#include <stdio.h>
#include <string.h>
#include <script-tree.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct{
	char* name;
	int fd[2];
}nodeData;

//module
static int parsArgment(char* str,const int size,char* argv[]);
static int popenRW(const char const * command,int* fd);

//command callback
static void help(int* argc,char* argv[]);
static void quit(int* argc,char* argv[]);
static void save(int* argc,char* argv[]);
static void load(int* argc,char* argv[]);
static void run(int* argc,char* argv[]);
static void connect(int* argc,char* argv[]);

static const Command commandList[] = {
	{"help",help},
	{"quit",quit},
	{"save",save},
	{"load",load},
	{"run",run},
	{"connect",connect}
};

static uint8_t exit_flag = 0;

void lunch(int* argc,char* argv[]){
	printf("lunch success.\n");
	
	//init terminal
	char inputData[1024];
	size_t inputDataSize = sizeof(inputDataSize);
	char* args[5];
	int oldInFlag = fcntl(STDIN_FILENO,F_GETFL);
	fcntl(STDIN_FILENO,F_SETFL,oldInFlag|O_NONBLOCK);
	putchar('>');

	//loop
	while(1){
		if(exit_flag)
			break;

		/*--------------nodeSystemBegin-------------*/

		/*---------------nodeSystemEnd--------------*/
		

		/*---------------terminalBegin--------------*/
		//get input
		if(fgets(inputData,sizeof(inputData),stdin) != NULL){
			inputData[strlen(inputData) - 1] = '\0';//replase \n to \0
			int count = parsArgment(inputData,sizeof(args)/sizeof(char*),args);
			
			//continue when no input
			if(!count)
				continue;

			//check comand input to call command function
			int i;
			int commandCount = sizeof(commandList)/sizeof(Command);
			for(i = 0;i < commandCount;i++){
				if(strcmp(args[0],commandList[i].argName) == 0){
					commandList[i].func(&count,args);
					break;
				}
			}
		
			if(i == commandCount)
				printf("%s is invalide token\n",args[0]);

			//terminal head
			putchar('>');
		}
		/*----------------terminalEnd---------------*/
	}

	//return
	putchar('\n');

	//reset
	fcntl(STDIN_FILENO,F_SETFD,oldInFlag);
}

static int parsArgment(char* str,const int size,char* argv[]){

	int i;
	for(i = 0;i < size;i++){
		while(*str == ' ')
			str++;

		if(*str == '\0')
			break;

		//set arg
		argv[i] = str;
		

		while(*str != '\0'){
			str++;

			if(*str == ' '){
				*str = '\0';
				str++;
			}
		}
	}

	return i;
}

static void help(int* argc,char* argv[]){
	printf("-------------------------Commands-------------------------\n"
		   " help -- display this text\n"
		   " quit -- quit program\n"
		   " save [-f savePath] -- save node relation to savePath\n"
		   " load [-f loadPath] -- load node relation from loadPath\n"
		   " run [-f programPath] -- run programPath as node\n"
		   " connect [NodeName.PortName ...] -- connect ports\n"
		   "----------------------------------------------------------\n");
}

static void quit(int* argc,char* argv[]){
	exit_flag = 1;
}

static void save(int* argc,char* argv[]){
}

static void load(int* argc,char* argv[]){
}

static void run(int* argc,char* argv[]){

}

static void connect(int* argc,char* argv[]){
}

