#include <lunch.h>
#include <linear_list.h>
#include <script-tree.h>

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

//module
static int parsArgment(char* str,int size,char* argv[]);
static int popenRWasNonBlock(const char const * command,char* argv[],int* fd);
static void fileRead(int fd,void* buffer,uint32_t size,uint32_t count);
static int fileReadWithTimeOut(int fd,void* buffer,uint32_t size,uint32_t count,uint32_t sec);
static int fileReadStrWithTimeOut(int fd,char* str,uint32_t len,uint32_t sec);
static int receiveNodeProperties(int fd,nodeData* node);

//readline function
char *command_generator(char* str,int state);
char **nodeSystem_completion(const char* str,int start,int end);

//command callback
static void help(int* argc,char* argv[]);
static void quit(int* argc,char* argv[]);
static void save(int* argc,char* argv[]);
static void load(int* argc,char* argv[]);
static void run(int* argc,char* argv[]);
static void connect(int* argc,char* argv[]);
static void clear(int* argc,char* argv[]);

static const Command commandList[] = {
	{"help",help,"help -- display this text"},
	{"quit",quit,"quit -- quit work space"},
	{"save",save,"save [-f savePath] -- save node relation to savePath"},
	{"load",load,"load [-f loadPath] -- load node relation from loadPath"},
	{"run" ,run ,"run [-f programPath] -- run programPath as node"},
	{"connect",connect,"connect [NodeName.PortName ...] -- connect ports"},
	{"clear",clear,"clear -- clear display"}
};

static nodeData** activeNodeList = NULL;
static uint8_t exit_flag = 0;
static const uint32_t _node_head = 0x83DFC690;
static const uint32_t _node_eof  = 0x85CBADEF;

void lunch(int* argc,char* argv[]){
	printf("lunch success.\n");

	//init nodeSystem
	activeNodeList  = LINEAR_LIST_CREATE(nodeData*);

	//init terminal
	char* args[5];
	rl_readline_name = "NodeSystem";
	rl_attempted_completion_function = nodeSystem_completion;
	int oldInFlag = fcntl(STDIN_FILENO,F_GETFL);
	fcntl(STDIN_FILENO,F_SETFL,oldInFlag|O_NONBLOCK);

	//loop
	while(1){
		if(exit_flag)
			break;

		/*--------------nodeSystemBegin-------------*/
		nodeData** itr;
		LINEAR_LIST_FOREACH(activeNodeList,itr){

		}
		/*---------------nodeSystemEnd--------------*/
		

		/*---------------terminalBegin--------------*/
		//get input
		char* inputLine = readline(">>>");
		if(inputLine != NULL && inputLine[0] != '\0'){
			int count = parsArgment(inputLine,sizeof(args)/sizeof(char*),args);
			
			//continue when no input
			if(!count)
				continue;

			//check comand input to call command function
			int i;
			int commandCount = sizeof(commandList)/sizeof(Command);
			for(i = 0;i < commandCount;i++){
				if(strcmp(args[0],commandList[i].name) == 0){
					commandList[i].func(&count,args);
					break;
				}
			}
		
			if(i == commandCount)
				printf("%s is invalide token\n",args[0]);

		}
		if(inputLine)
			free(inputLine);
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

static int popenRWasNonBlock(const char const * command,char* argv[],int* fd){

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

		//set nonblock
		fcntl(fd[0],F_SETFL,fcntl(fd[0],F_GETFL) | O_NONBLOCK);
		fcntl(fd[1],F_SETFL,fcntl(fd[1],F_GETFL) | O_NONBLOCK);
		fcntl(fd[2],F_SETFL,fcntl(fd[2],F_GETFL) | O_NONBLOCK);

		return process;
	}
}

static void fileRead(int fd,void* buffer,uint32_t size,uint32_t count){
	uint64_t bufferSize = size * count;
	int64_t readCount;
	uint32_t readSize = 0;
	
	do{
		readCount = read(fd,buffer + readSize,bufferSize);
		if(readCount > 0)
			readSize += readCount;
	}while(readSize != bufferSize);
}

static int fileReadWithTimeOut(int fd,void* buffer,uint32_t size,uint32_t count,uint32_t sec){
	uint64_t bufferSize = size * count;
	int64_t readCount;
	uint32_t readSize = 0;
	
	time_t target = time(NULL) + sec;

	do{
		readCount = read(fd,buffer + readSize,bufferSize);
		if(readCount > 0)
			readSize += readCount;
	}while(readSize != bufferSize && target > time(NULL));

	return readSize;
}

static int fileReadStrWithTimeOut(int fd,char* str,uint32_t len,uint32_t sec){
	uint32_t readSize = 0;
	
	time_t target = time(NULL) + sec;

	do{
		if(read(fd,&str[readSize],1) == 1){
			readSize ++;
		}
	}while((readSize == 0 || str[readSize-1] != '\0') && target > time(NULL) && len > readSize);

	return readSize;
}

static int receiveNodeProperties(int fd,nodeData* node){
	char recvBuffer[1024];
	
	//recive header
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(_node_head),1,10) != sizeof(_node_head)){
		fprintf(stderr,"Header receive sequrnce time out\n");
		return -1;
	}else if(((uint32_t*)recvBuffer)[0] != _node_head){
		fprintf(stderr,"received header is invalid\n");
		return -2;
	}
	fprintf(stdout,"Header receive succsee\n");

	//recive pipe count
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(uint16_t),1,10) != sizeof(uint16_t)){
		fprintf(stderr,"Pipe count receive sequrnce time out\n");
		return -1;
	}	
	node->pipeCount = ((uint16_t*)recvBuffer)[0];
	node->pipes = malloc(sizeof(nodePipe)*node->pipeCount);
	memset(node->pipes,0,sizeof(nodePipe)*node->pipeCount);
	fprintf(stdout,"Pipe count : %d\n",(int)node->pipeCount);
	
	//recive pipe
	uint16_t i;
	for(i = 0;i < node->pipeCount;i++){
		//type
		if(fileReadWithTimeOut(fd,&node->pipes[i].type,sizeof(uint8_t),1,10) != sizeof(uint8_t)){
			fprintf(stderr,"Pipe type receive sequrnce time out\n");
			return -1;
		}	

		//unit
		if(fileReadWithTimeOut(fd,&node->pipes[i].unit,sizeof(uint8_t),1,10) != sizeof(uint8_t)){
			fprintf(stderr,"Pipe unit receive sequrnce time out\n");
			return -1;
		}	
		
		//length
		if(fileReadWithTimeOut(fd,&node->pipes[i].length,sizeof(uint16_t),1,10) != sizeof(uint16_t)){
			fprintf(stderr,"Unit length receive sequrnce time out\n");
			return -1;
		}	
		
		//name
		uint32_t len = fileReadStrWithTimeOut(fd,recvBuffer,sizeof(recvBuffer),10);
		if(len == 0 || recvBuffer[len - 1] != '\0'){
			fprintf(stderr,"Unit length receive sequrnce time out\n");
			return -1;
		}
		node->pipes[i].pipeName = malloc(len);
		strcpy(node->pipes[i].pipeName,recvBuffer);
	
		fprintf(stdout,	
				"--------------------------------------\n"
				"PipeName: %s\n"
				"PipeType: %s\n"
				"PipeUnit: %s\n"
				"ArraySize: %d\n"
				"--------------------------------------\n",
				node->pipes[i].pipeName,
				NODE_PIPE_TYPE_STR[node->pipes[i].type],
				NODE_DATA_UNIT_STR[node->pipes[i].unit],
				node->pipes[i].length);
	}

	//recive eof
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(_node_eof),1,10) != sizeof(_node_eof)){
		fprintf(stderr,"EOF receive sequrnce time out\n");
		return -1;
	}else if(((uint32_t*)recvBuffer)[0] != _node_eof){
		fprintf(stderr,"received EOF is invalid\n");
		return -2;
	}
	fprintf(stdout,"Header EOF succsee\n");
	return 0;
}

//readline func
char *command_generator(char* str,int status){
	static int list_index, len;
	char *name;

	if(!state){
		list_index = 0;
		len = strlen (str);
	}

	while(name = commandList[list_index].name)
	{
		list_index++;

		if (strncmp (name, str, len) == 0)
			return (dupstr(name));
	}

	return ((char *)NULL);
}

char **nodeSystem_completion(const char* str,int start,int end){
	if(!start)
		return rl_completion_matches(str,command_generator);

	return NULL;
}

//command callback
static void help(int* argc,char* argv[]){
	fprintf(stdout,"-------------------------Commands-------------------------\n");
	int i;
	int commandCount = sizeof(commandList)/sizeof(Command);
	for(i = 0;i < commandCount;i++){
		fprintf(stdout," %s\n",commandList[i].man);
	}
	fprintf(stdout,"----------------------------------------------------------\n");
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
	int pid = popenRWasNonBlock(argv[1],&argv[1],data->fd);
	if(pid < 0){
		perror(__func__);
		exit(0);
	}
	
	//load properties
	if(!receiveNodeProperties(data->fd[0],data))
		LINEAR_LIST_PUSH(activeNodeList,data);
}

static void connect(int* argc,char* argv[]){
}

static void clear(int* argc,char* argv[]){
	system("clear");
}
