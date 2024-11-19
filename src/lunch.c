#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <script-tree.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <linear_list.h>

typedef struct{
	char* name;
	int fd[3];
}nodeData;

//module
static int parsArgment(char* str,int size,char* argv[]);
static int popenRW(const char const * command,char* argv[],int* fd);

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

static nodeData** activeNodeList = NULL;

static uint8_t exit_flag = 0;

void lunch(int* argc,char* argv[]){
	printf("lunch success.\n");

	//init nodeSystem
	activeNodeList  = LINEAR_LIST_CREATE(nodeData);

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

		/*--------------nodeSystemBegin-------------*/a
		nodeData** itr;
		LINEAR_LIST_FOREACH(activeNodeList,itr){

		}
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

static int parsArgment(char* str,int size,char* argv[]){
	size--;

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
				break;
			}
		}
	}

	argv[i] = NULL;
	return i;
}

static int popenRW(const char const * command,char* argv[],int* fd){

	int pipeTx[2];
	int pipeRx[2];
	int pipeErr[2];
	//create pipe
	if(pipe(pipeTx) < 0 || pipe(pipeRx) < 0 || pipe(pipeErr) < 0)
		return -1;

	//fork
	int process = fork();
	if(process == -1)
		return -1;

 	if(process == 0)
	{//child
	 	//dup pipe
		close(pipeTx[1]);
		close(pipeRx[0]);
		close(pipeErr[0]);
		dup2(pipeTx[0], STDIN_FILENO);
		dup2(pipeRx[1], STDOUT_FILENO);
		dup2(pipeErr[1], STDERR_FILENO);
		close(pipeTx[0]);
		close(pipeRx[1]);
		close(pipeErr[1]);
		
		execvp(command,argv);

		exit(EXIT_SUCCESS);
	}
	else
	{//parent
		//close pipe
		close(pipeTx[0]);
		close(pipeRx[1]);
		close(pipeErr[1]);
		fd[0] = pipeRx[0];
		fd[1] = pipeTx[1];
		fd[2] = pipeErr[0];

		return process;
	}
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
	//init struct
	nodeData* data = malloc(sizeof(nodeData));
	memset(data,0,sizeof(nodeData));
	data->name = malloc(strlen(argv[1])+1);
	strcpy(data->name,argv[1]);

	//execute program
	int pid = popenRW(argv[1],&argv[1],data->fd);
	if(pid < 0){
		perror(__func__);
		exit(0);
	}

	//transmit setting data
	
	
	
	LINEAR_LIST_PUSH(activeNodeList,data);
}

static void connect(int* argc,char* argv[]){
}

