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
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

//module
static int parsArgment(char* str,int size,char* argv[]);
static int popenRWasNonBlock(const char const * command,char* argv[],int* fd);
static void fileRead(int fd,void* buffer,uint32_t size,uint32_t count);
static int fileReadWithTimeOut(int fd,void* buffer,uint32_t size,uint32_t count,uint32_t sec);
static int fileReadStrWithTimeOut(int fd,char* str,uint32_t len,uint32_t sec);
static int receiveNodeProperties(int fd,nodeData* node);
static int nodeBegin(nodeData* node);

//readline function
char *command_generator(const char* str,int state);
char **nodeSystem_completion(const char* str,int start,int end);

//command callback
static void s_help(int* argc,char* argv[]);
static void s_quit(int* argc,char* argv[]);
static void s_save(int* argc,char* argv[]);
static void s_load(int* argc,char* argv[]);
static void s_run(int* argc,char* argv[]);
static void s_connect(int* argc,char* argv[]);
static void s_clear(int* argc,char* argv[]);

static const Command commandList[] = {
	{"help",s_help,"help -- display this text"},
	{"quit",s_quit,"quit -- quit work space"},
	{"save",s_save,"save [-f savePath] -- save node relation to savePath"},
	{"load",s_load,"load [-f loadPath] -- load node relation from loadPath"},
	{"run" ,s_run ,"run [-f programPath] -- run programPath as node"},
	{"connect",s_connect,"connect [NodeName.PortName ...] -- connect ports"},
	{"clear",s_clear,"clear -- clear display"}
};

static nodeData** activeNodeList = NULL;
static nodeData** inactiveNodeList = NULL;
static uint8_t no_log = 0;
static uint8_t exit_flag = 0;
static const uint32_t _node_init_head = 0x83DFC690;
static const uint32_t _node_init_eof  = 0x85CBADEF;
static const uint32_t _node_begin_head = 0x9067F3A2;
static const uint32_t _node_begin_eof  = 0x910AC8BB;

void lunch(int* argc,char* argv[]){
	printf("lunch success.\n");

	//find option
	int i;
	for(i = 0;argv[i] != NULL;i++){
		if(strcmp(argv[i],"-noLog") == 0){
			no_log = 1;
		}
	}

	//init nodeSystem
	activeNodeList  = LINEAR_LIST_CREATE(nodeData*);
	inactiveNodeList  = LINEAR_LIST_CREATE(nodeData*);

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
		LINEAR_LIST_FOREACH(inactiveNodeList,itr){
			int res = nodeBegin(*itr);
			if(res){
				if(res < 0){
					kill((*itr)->pid,SIGTERM);	
					LINEAR_LIST_ERASE(itr);
					break;
				}
				else{
					nodeData* data = *itr;
					LINEAR_LIST_ERASE(itr);
					LINEAR_LIST_PUSH(activeNodeList,data);
					break;
				}
					
			}
		}

		LINEAR_LIST_FOREACH(activeNodeList,itr){
			
		}
		/*---------------nodeSystemEnd--------------*/
		

		/*---------------terminalBegin--------------*/
		//get input
		char* inputLine = readline(">>>");
		if(inputLine != NULL && inputLine[0] != '\0'){
			//add history
			add_history(inputLine);

			int count = parsArgment(inputLine,sizeof(args)/sizeof(char*),args);

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

static int nodeBegin(nodeData* node){
	uint32_t header_buffer;
	
	int count = fileReadWithTimeOut(node->fd[0],&header_buffer,sizeof(uint32_t),1,1);

	if(!count)
		return 0;

	if(count < sizeof(uint32_t)){
		int count = fileReadWithTimeOut(node->fd[0],
				((char*)&header_buffer)+count,sizeof(uint32_t)-count,1,1000);
	}

	if(count != sizeof(uint32_t)){
		int cursol = rl_point + 3;
		fprintf(stderr,"\e[1E[%s]failed recive header\e[1A\e[%dC\n",node->name,cursol);
		return -1;
	}

	

	return 1;
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

static int fileReadWithTimeOut(int fd,void* buffer,uint32_t size,uint32_t count,uint32_t msec){
	uint64_t bufferSize = size * count;
	int64_t readCount;
	uint32_t readSize = 0;
 	struct timespec target,spec;

    clock_gettime(CLOCK_REALTIME, &target);
    target.tv_nsec += 1000000LL * msec;
	target.tv_sec += target.tv_nsec / 1000000000;
	target.tv_nsec %= 1000000000;
	

	do{
    	clock_gettime(CLOCK_REALTIME, &spec);
		readCount = read(fd,buffer + readSize,bufferSize);
		if(readCount > 0)
			readSize += readCount;
	}while(readSize != bufferSize && !(target.tv_sec < spec.tv_sec && target.tv_nsec < spec.tv_nsec));

	return readSize;
}

static int fileReadStrWithTimeOut(int fd,char* str,uint32_t len,uint32_t msec){
	uint32_t readSize = 0;
 	struct timespec target,spec;

    clock_gettime(CLOCK_REALTIME, &target);
    target.tv_nsec += 1000000LL * msec;
	target.tv_sec += target.tv_nsec / 1000000000;
	target.tv_nsec %= 1000000000;
	
	do{
    	clock_gettime(CLOCK_REALTIME, &spec);
		if(read(fd,&str[readSize],1) == 1){
			readSize ++;
		}
	}while((readSize == 0 || str[readSize-1] != '\0') && !(target.tv_sec < spec.tv_sec && target.tv_nsec < spec.tv_nsec));

	return readSize;
}

static int receiveNodeProperties(int fd,nodeData* node){
	char recvBuffer[1024];
	
	//recive header
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(_node_init_head),1,1000) != sizeof(_node_init_head)){
		fprintf(stderr,"Header receive sequrnce time out\n");
		return -1;
	}else if(((uint32_t*)recvBuffer)[0] != _node_init_head){
		fprintf(stderr,"received header is invalid\n");
		return -2;
	}
	fprintf(stdout,"Header receive succsee\n");

	//recive pipe count
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(uint16_t),1,1000) != sizeof(uint16_t)){
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
		if(fileReadWithTimeOut(fd,&node->pipes[i].type,sizeof(uint8_t),1,1000) != sizeof(uint8_t)){
			fprintf(stderr,"Pipe type receive sequrnce time out\n");
			return -1;
		}	

		//unit
		if(fileReadWithTimeOut(fd,&node->pipes[i].unit,sizeof(uint8_t),1,1000) != sizeof(uint8_t)){
			fprintf(stderr,"Pipe unit receive sequrnce time out\n");
			return -1;
		}	
		
		//length
		if(fileReadWithTimeOut(fd,&node->pipes[i].length,sizeof(uint16_t),1,1000) != sizeof(uint16_t)){
			fprintf(stderr,"Unit length receive sequrnce time out\n");
			return -1;
		}	
		
		//name
		uint32_t len = fileReadStrWithTimeOut(fd,recvBuffer,sizeof(recvBuffer),1000);
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
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(_node_init_eof),1,1000) != sizeof(_node_init_eof)){
		fprintf(stderr,"EOF receive sequrnce time out\n");
		return -1;
	}else if(((uint32_t*)recvBuffer)[0] != _node_init_eof){
		fprintf(stderr,"received EOF is invalid\n");
		return -2;
	}
	fprintf(stdout,"Header EOF succsee\n");
	return 0;
}

//readline func
char *command_generator(const char* str,int status){
	static int list_index, len;

	if(!status){
		list_index = 0;
		len = strlen (str);
	}

	char* name;
	int commandCount = (sizeof(commandList)/sizeof(Command));
	while(list_index < commandCount){
		name = commandList[list_index].name;
		list_index++;

		if (strncmp(name, str, len) == 0){
			char* cpStr = malloc(strlen(name)+1);

			if(cpStr){
				strcpy(cpStr,name);
				return cpStr;
			}
		}
	}

	return ((char *)NULL);
}

char **nodeSystem_completion(const char* str,int start,int end){
	if(!start)
		return rl_completion_matches(str,command_generator);

	return NULL;
}

//command callback
static void s_help(int* argc,char* argv[]){
	fprintf(stdout,"-------------------------Commands-------------------------\n");
	int i;
	int commandCount = sizeof(commandList)/sizeof(Command);
	for(i = 0;i < commandCount;i++){
		fprintf(stdout," %s\n",commandList[i].man);
	}
	fprintf(stdout,"----------------------------------------------------------\n");
}

static void s_quit(int* argc,char* argv[]){
	exit_flag = 1;
}

static void s_save(int* argc,char* argv[]){
}

static void s_load(int* argc,char* argv[]){
}

static void s_run(int* argc,char* argv[]){
	//init struct
	nodeData* data = malloc(sizeof(nodeData));
	memset(data,0,sizeof(nodeData));
	data->name = malloc(strlen(argv[1])+1);
	strcpy(data->name,argv[1]);

	//execute program
	data->pid = popenRWasNonBlock(argv[1],&argv[1],data->fd);
	if(data->pid < 0){
		perror(__func__);
		exit(0);
	}
	
		int cursol = rl_point + 3;
		fprintf(stderr,"\e[1E[%s]failed recive header\n\e[2A\e[%dC",data->name,cursol);
	//load properties
	if(!receiveNodeProperties(data->fd[0],data))
		LINEAR_LIST_PUSH(activeNodeList,data);
	else
		kill(data->pid,SIGTERM);	
}

static void s_connect(int* argc,char* argv[]){
}

static void s_clear(int* argc,char* argv[]){
	system("clear");
}
