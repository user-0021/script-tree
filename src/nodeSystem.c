#include <nodeSystem.h>
#include <linear_list.h>

#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

//local func
static void nodeSystemLoop();
static int nodeBegin(nodeData* node);
static void nodeDeleate(nodeData* node);
static int receiveNodeProperties(int fd,nodeData* node);
static int popenRWasNonBlock(const char const * command,int* fd);

static void fileRead(int fd,void* buffer,uint32_t size,uint32_t count);
static int fileReadWithTimeOut(int fd,void* buffer,uint32_t size,uint32_t count,uint32_t sec);
static int fileReadStrWithTimeOut(int fd,char* str,uint32_t len,uint32_t sec);
static char* getRealTimeStr(const char* fmt);

static nodeData* pipeAddNode();

//state enum
enum _pipeHead{
	PIPE_ADD_NODE = 0
};

//const value
static const char logRootPath[] = "Logs";

//global value
static int pid;
static int fd[2];
static FILE* logFile;
static char* logFolder;
static time_t timeZone;
static uint8_t no_log = 0;
static nodeData** activeNodeList = NULL;
static nodeData** inactiveNodeList = NULL;

int nodeSystemInit(uint8_t isNoLog){
	int in[2],out[2];
	
	//create logDirPath
	char logFilePath[1024];

	strcpy(logFilePath,logRootPath);
	strcat(logFilePath,"/");
	strcat(logFilePath,getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
	

	//createDir
	if(!no_log){
		logFolder = malloc(strlen(logFilePath)+1);
		strcpy(logFolder,logFilePath);
		mkdir(logRootPath,0777);
		if(mkdir(logFilePath,0777) != 0){
			perror(__func__);
			return -1;
		}
	}
	
	
	// open pipe 
	if(pipe(in) != 0 && pipe(out) != 0){
		perror(__func__);
		return -1;
	}

	//set nonblock
	fcntl(in[0] ,F_SETFL,fcntl(in[0] ,F_GETFL) | O_NONBLOCK);
	fcntl(in[1] ,F_SETFL,fcntl(in[1] ,F_GETFL) | O_NONBLOCK);
	fcntl(out[0],F_SETFL,fcntl(out[0],F_GETFL) | O_NONBLOCK);
	fcntl(out[1],F_SETFL,fcntl(out[1],F_GETFL) | O_NONBLOCK);

		
	// fork program
	pid = fork();
	if(pid == -1){
		perror(__func__);
		return -1;
	}

	if(pid != 0){
		//parent
		fd[0] = out[0];
		fd[1] = in[1];
		close(out[1]);
		close(in[0]);
	}else{
		//child
		fd[0] = in[0];
		fd[1] = out[1];
		close(out[0]);
		close(in[1]);

		//init list
		activeNodeList = LINEAR_LIST_CREATE(nodeData*);
		inactiveNodeList = LINEAR_LIST_CREATE(nodeData*);


		if(!no_log){
			//create log file path
			char logFilePath[1024];
			strcpy(logFilePath,logFolder);
			strcat(logFilePath,"/NodeSystem.txt");
			logFile = fopen(logFilePath,"w");

			//failed open logFile
			if(logFile == NULL){
				exit(-1);
			}

			fprintf(logFile,"%s:nodeSystem is activate.\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
			fflush(logFile);
		}

		//loop
		int parent = getppid();
		while(kill(parent,0) == 0)
			nodeSystemLoop();


		if(!no_log)
			fclose(logFile);
		exit(0);
	}

	return 0;
}

int nodeSystemAdd(char* path,char** args){
	if(path == NULL)
		return -1;
	
	//count args
	uint16_t c = 0;
	while(args[c] != NULL)
		c++;

	//send message head
	uint8_t head = PIPE_ADD_NODE;
	write(fd[1],&head,sizeof(head));
	
	//send execute path
	uint16_t pipeLength = strlen(path) + 1;
	write(fd[1],&pipeLength,sizeof(pipeLength));
	write(fd[1],path,pipeLength);

	//send args count
	write(fd[1],&c,sizeof(c));

	//send args
	int i;
	for(i = 0;i < c;i++){
		uint16_t strLen = strlen(args[i]) + 1; 
		write(fd[1],&strLen,sizeof(strLen));
		write(fd[1],args[i],strLen);
	}

	return 0;
}

static void nodeSystemLoop(){
	uint8_t head;
	//check inactive nodes
	nodeData** itr;
	LINEAR_LIST_FOREACH(inactiveNodeList,itr){
		int ret = nodeBegin(*itr);
		if(!no_log)
			fflush(logFile);
		if(ret == 0){
			nodeData* data = *itr;
			LINEAR_LIST_ERASE(itr);

			LINEAR_LIST_PUSH(activeNodeList,data);
		}else if(ret < 0){
			//kill
			kill((*itr)->pid,SIGTERM);
			
			//deleate node
			nodeDeleate(*itr);

			//deleate from list
			LINEAR_LIST_ERASE(itr);
		}
	}

	//check active node
	LINEAR_LIST_FOREACH(activeNodeList,itr){
		if(kill((*itr)->pid,0)){
			//process is terminated
			//deleate node
			nodeDeleate(*itr);
			//deleate from list
			LINEAR_LIST_ERASE(itr);
		}else{
			int i;
			for(i = 0;i < (*itr)->pipeCount;i++){
				if((*itr)->pipes[i].type != NODE_IN){
					
				}
			}
		}
	}

	//message from parent 
	if(read(fd[0],&head,sizeof(head)) == 1){
		//recive from parent
		switch(head){
			case PIPE_ADD_NODE:{//add node			

				nodeData* data = pipeAddNode();	

				//execute program
				data->pid = popenRWasNonBlock(data->filePath,data->fd);
				if(data->pid < 0){
					perror(__func__);
					exit(0);
				}
				
				//load properties
				if(receiveNodeProperties(data->fd[0],data))
					kill(data->pid,SIGTERM);
				else
					LINEAR_LIST_PUSH(inactiveNodeList,data);

				break;
			}
			default:
		}
	}

	if(!no_log)
		fflush(logFile);
}

static int popenRWasNonBlock(const char const * command,int* fd){

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
		
		execl(command,command,NULL);

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
	
	int ret = fileReadWithTimeOut(node->fd[0],&header_buffer,sizeof(uint32_t),1,1);

	if(ret == 0){
		fprintf(logFile,"%s:[%s]Rloop\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
		return 1;
	}
	else if((ret != sizeof(uint32_t))){
		if(!no_log)
			fprintf(logFile,"%s:[%s]Recive header sequence timeout\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
		return -1;
	}else if(header_buffer != _node_begin_head){
		if(!no_log)
			fprintf(logFile,"%s:[%s]Failed recive header\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
		return -2;
	}

	//give pipe
	int i,f = 0;
	for(i = 0;i < node->pipeCount;i++){
		switch(node->pipes[i].type){
			case NODE_IN:{
				int in[2];
				if(pipe(in) != 0){
					f = 1;
				}else{
					write(node->fd[1],&in[0],sizeof(int));
					node->pipes[i].fd[1] = in[1];
				}
				break;
			}			
			case NODE_OUT:{
				int out[2];
				if(pipe(out) != 0){
					f = 1;
				}else{
					write(node->fd[1],&out[1],sizeof(int));
					node->pipes[i].fd[0] = out[0];
					
					if(!no_log){
						//create log file path
						char logFilePath[1024];
						strcpy(logFilePath,logFolder);
						strcat(logFilePath,"/");
						strcat(logFilePath,node->name);
						strcat(logFilePath,"_");
						strcat(logFilePath,node->pipes[i].pipeName);
						strcat(logFilePath,".txt");
						node->pipes[i].log = fopen(logFilePath,"w");

						//failed open logFile
						if(node->pipes[i].log == NULL){
							fprintf(logFile,"%s:[%s]failed create logfile.\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
						}
					}
				}
				break;
			}			
			case NODE_IN_OUT:{
				int in[2];
				int out[2];
				if(pipe(in) != 0 || pipe(out) != 0){
					f = 1;
				}else{
					write(node->fd[1],&in[0],sizeof(int));
					write(node->fd[1],&out[1],sizeof(int));
					node->pipes[i].fd[0] = out[0];
					node->pipes[i].fd[1] = in[1];

					if(!no_log){
						//create log file path
						char logFilePath[1024];
						strcpy(logFilePath,logFolder);
						strcat(logFilePath,"/");
						strcat(logFilePath,node->name);
						strcat(logFilePath,"_");
						strcat(logFilePath,node->pipes[i].pipeName);
						strcat(logFilePath,".txt");
						node->pipes[i].log = fopen(logFilePath,"w");

						//failed open logFile
						if(node->pipes[i].log == NULL){
							fprintf(logFile,"%s:[%s]failed create logfile.\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
						}
					}
				}
				break;
			}
		}
		if (f == 1){
			perror(node->name);
			return -1;
		}
	}

	if((fileReadWithTimeOut(node->fd[0],&header_buffer,sizeof(uint32_t),1,1000) != sizeof(uint32_t))){
		if(!no_log)
			fprintf(logFile,"%s:[%s]Recive eof sequence timeout\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
		return -1;
	}else if(header_buffer != _node_begin_eof){
		if(!no_log)
			fprintf(logFile,"%s:[%s]failed recive eof\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
		return -2;
	}

	if(!no_log)
		fprintf(logFile,"%s:[%s]node is activate\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);
	return 0;
}

static void nodeDeleate(nodeData* node){

	//log
	if(!no_log)
		fprintf(logFile,"%s:[%s]node is deleate\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),node->name);

	//releace mem
	int i;
	for(i = 0;i < node->pipeCount;i++){
		free(node->pipes[i].pipeName);
	}
	
	free(node->pipes);
	free(node->name); 
	free(node->filePath); 
	free(node);

}

static void fileRead(int fd,void* buffer,uint32_t size,uint32_t count){
	uint64_t bufferSize = size * count;
	int64_t readCount;
	uint32_t readSize = 0;
	
	do{
		readCount = read(fd,buffer + readSize,bufferSize - readSize);
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
		readCount = read(fd,buffer + readSize,bufferSize - readSize);
		if(readCount > 0)
			readSize += readCount;
	}while(readSize != bufferSize && !(target.tv_sec <= spec.tv_sec && target.tv_nsec <= spec.tv_nsec));

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
		if(!no_log)
			fprintf(logFile,"%s:Header receive sequence time out\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
		return -1;
	}else if(((uint32_t*)recvBuffer)[0] != _node_init_head){
		if(!no_log)
			fprintf(logFile,"%s:Received header is invalid\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
		return -2;
	}
	if(!no_log)
		fprintf(logFile,"%s:Header receive succsee\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));

	//recive pipe count
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(uint16_t),1,1000) != sizeof(uint16_t)){
		if(!no_log)
			fprintf(logFile,"%s:Pipe count receive sequence time out\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
		return -1;
	}	
	node->pipeCount = ((uint16_t*)recvBuffer)[0];
	node->pipes = malloc(sizeof(nodePipe)*node->pipeCount);
	memset(node->pipes,0,sizeof(nodePipe)*node->pipeCount);
	if(!no_log)
		fprintf(logFile,"%s:Pipe count %d\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"),(int)node->pipeCount);
	
	//recive pipe
	if(!no_log)
		fprintf(logFile,"%s:Pipe receive sequence begin\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
	uint16_t i;
	for(i = 0;i < node->pipeCount;i++){
		//type
		if(fileReadWithTimeOut(fd,&node->pipes[i].type,sizeof(uint8_t),1,1000) != sizeof(uint8_t)){
			if(!no_log)
				fprintf(logFile,"%s:Pipe type receive sequence time out\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
			return -1;
		}	

		//unit
		if(fileReadWithTimeOut(fd,&node->pipes[i].unit,sizeof(uint8_t),1,1000) != sizeof(uint8_t)){
			if(!no_log)
				fprintf(logFile,"%s:Pipe unit receive sequence time out\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
			return -1;
		}	
		
		//length
		if(fileReadWithTimeOut(fd,&node->pipes[i].length,sizeof(uint16_t),1,1000) != sizeof(uint16_t)){
			if(!no_log)
				fprintf(logFile,"%s:Unit length receive sequence time out\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
			return -1;
		}	
		
		//name
		uint32_t len = fileReadStrWithTimeOut(fd,recvBuffer,sizeof(recvBuffer),1000);
		if(len == 0 || recvBuffer[len - 1] != '\0'){
			if(!no_log)
				fprintf(logFile,"%s:Unit length receive sequence time out\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
			return -1;
		}
		node->pipes[i].pipeName = malloc(len);
		strcpy(node->pipes[i].pipeName,recvBuffer);
	
		if(!no_log)
			fprintf(logFile,	
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
	if(!no_log)
		fprintf(logFile,"%s:Pipe receive sequence end\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));

	//recive eof
	if(fileReadWithTimeOut(fd,recvBuffer,sizeof(_node_init_eof),1,1000) != sizeof(_node_init_eof)){
		if(!no_log)
			fprintf(logFile,"%s:EOF receive sequence time out\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
		return -1;
	}else if(((uint32_t*)recvBuffer)[0] != _node_init_eof){
		if(!no_log)
			fprintf(logFile,"%s:received EOF is invalid\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));
		return -2;
	}
	if(!no_log)
		fprintf(logFile,"%s:Header EOF succsee\n",getRealTimeStr("%Y-%m-%d-(%a)-%H:%M:%S"));

	return 0;
}

static char* getRealTimeStr(const char* fmt){
	static char timeStr[64];
	static time_t befor;

	struct timespec spec;
    struct tm _tm;
	clock_gettime(CLOCK_REALTIME, &spec);

	if(spec.tv_sec == befor)
		return timeStr;

	gmtime_r(&spec.tv_sec,&_tm);
	strftime(timeStr,sizeof(timeStr),fmt,&_tm);

	befor = spec.tv_sec;
	return timeStr;
}


static nodeData* pipeAddNode(){
	//init struct
	nodeData* data = malloc(sizeof(nodeData));
	memset(data,0,sizeof(nodeData));

	//get path
	uint16_t pathLength;
	fileRead(fd[0],&pathLength,sizeof(pathLength),1);
	data->filePath = malloc(pathLength);
	fileRead(fd[0],data->filePath,pathLength,1);
	data->name = strrchr(data->filePath,'/');
	if(data->name)
		data->name++;
	else
		data->name = data->filePath;

	//args count
	uint16_t argsCount;
	fileRead(fd[0],&argsCount,sizeof(argsCount),1);

	//load args
	char** args = malloc(sizeof(char*)*argsCount);
	int i;
	for(i = 0;i < argsCount;i++){
		uint16_t len;
		fileRead(fd[0],&len,sizeof(len),1);
		args[i] = malloc(len);

		fileRead(fd[0],args[i],len,1);
	}

	//do args
	for(i = 0;i < (argsCount-1);i++){
		if(strcmp(args[i],"-name") == 0){
			free(args[i++]);
			data->name = args[i];
		}else{
			free(args[i]);
		}
	}

	//free
	free(args);

	return data;
}