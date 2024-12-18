#include <lunch.h>
#include <nodeSystem.h>
#include <linear_list.h>
#include <script-tree.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

//module
static int parsArgment(char* str,int size,char* argv[]);

//readline function
void process_input(char* str);
char *command_generator(const char* str,int state);
char **nodeSystem_completion(const char* str,int start,int end);

//command callback
static void s_help(int* argc,char* argv[]);
static void s_quit(int* argc,char* argv[]);
static void s_save(int* argc,char* argv[]);
static void s_load(int* argc,char* argv[]);
static void s_run(int* argc,char* argv[]);
static void s_connect(int* argc,char* argv[]);
static void s_disconnect(int* argc,char* argv[]);
static void s_list(int* argc,char* argv[]);
static void s_clear(int* argc,char* argv[]);
static void s_const(int* argc,char* argv[]);

static const Command commandList[] = {
	{"help",s_help,"help -- display this text"},
	{"quit",s_quit,"quit -- quit work space"},
	{"save",s_save,"save [-f savePath] -- save node relation to savePath"},
	{"load",s_load,"load [-f loadPath] -- load node relation from loadPath"},
	{"run" ,s_run ,"run [-f programPath] -- run programPath as node"},
	{"connect",s_connect,"connect [inNodeName] [inPipeName] [outNodeName] [outPipeName] -- connect ports"},
	{"disconnect",s_disconnect,"disconnect [inNodeName] [inPipeName]  -- disconnect ports"},
	{"list" ,s_list,"list -- show node list"},
	{"clear",s_clear,"clear -- clear display"},
	{"const",s_const,"[set/get] [constNodeName] [constPipeName] -- set/get const value"}
};

static uint8_t exit_flag = 0;

void lunch(int* argc,char* argv[]){
	printf("lunch success.\n");

	int no_log = 0;
	//find option
	int i;
	for(i = 0;argv[i] != NULL;i++){
		if(strcmp(argv[i],"-noLog") == 0){
			no_log = 1;
		}
	}

	//init nodeSystem
	if(nodeSystemInit(no_log) < 0){
		exit(1);
	}

	//init terminal
	rl_readline_name = "NodeSystem";
	rl_attempted_completion_function = nodeSystem_completion;
	int oldInFlag = fcntl(STDIN_FILENO,F_GETFL);
	fcntl(STDIN_FILENO,F_SETFL,oldInFlag|O_NONBLOCK);
	rl_callback_handler_install(">>>",process_input);

	//loop
	while(1){
		if(exit_flag)
			break;

		/*---------------terminalBegin--------------*/
		//get input
		rl_callback_read_char();	

		/*----------------terminalEnd---------------*/
	}

	rl_callback_handler_remove();
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

//readline func
void process_input(char* str){
	char* args[10];
	char* inputLine = str;
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

	rl_callback_handler_remove();
	rl_callback_handler_install(">>>",process_input);
}


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

char *node_generator(const char* str,int status){
	static int list_index, len;

	if(!status){
		list_index = 0;
		len = strlen (str);
	}

	//get list
	int nodeCount;
	char** names = nodeSystemGetNodeNameList(&nodeCount);

	//check match
	char* name;
	char* cpStr = NULL;
	while(list_index < nodeCount){
		name = names[list_index];
		list_index++;

		if (strncmp(name, str, len) == 0){
			cpStr = malloc(strlen(name)+1);

			if(cpStr){
				strcpy(cpStr,name);
				break;
			}
		}
	}

	//free list
	int i;
	for(i = 0;i < nodeCount;i++){
		free(names[i]);
	}
	free(names);

	return cpStr;
}

static char* selectedNodeName;
char *pipe_generator(const char* str,int status){
	static int list_index, len;

	if(!status){
		list_index = 0;
		len = strlen (str);
	}

	//get list
	int pipeCount;
	char** names = nodeSystemGetPipeNameList(selectedNodeName,&pipeCount);

	//check match
	char* name;
	char* cpStr = NULL;
	while(list_index < pipeCount){
		name = names[list_index];
		list_index++;

		if (strncmp(name, str, len) == 0){
			cpStr = malloc(strlen(name)+1);

			if(cpStr){
				strcpy(cpStr,name);
				break;
			}
		}
	}

	//free list
	int i;
	for(i = 0;i < pipeCount;i++){
		free(names[i]);
	}
	free(names);

	return cpStr;
}

char *set_get_generator(const char* str,int status){
	static int list_index, len;
	static const char* names[2] = {"set","get"};

	if(!status){
		list_index = 0;
		len = strlen (str);
	}

	//check match
	const char* name;
	while(list_index < (sizeof(names)/sizeof(char*))){
		name = names[list_index];
		list_index++;

		if (strncmp(name, str, len) == 0){
			char* cpStr = malloc(strlen(name)+1);

			if(cpStr){
				strcpy(cpStr,name);
				return cpStr;
			}
		}
	}
	return NULL;
}

char **nodeSystem_completion(const char* str,int start,int end){
	char** ret = NULL;

	if(!start)
		ret = rl_completion_matches(str,command_generator);
	else{
		//get line
		char* cpyLine = malloc(strlen(rl_line_buffer)+1);
		strcpy(cpyLine,rl_line_buffer);

		//parse argments
		char* args[10];
		int count = parsArgment(cpyLine,sizeof(args)/sizeof(char*),args);
		
		//get cursol index
		int i;
		int cursoledArgment = -1;
		for(i = 0;i < count;i++){
			if((args[i] - cpyLine) == start){
				cursoledArgment = i;
				break;
			}
		}
		if(cursoledArgment == -1)
			cursoledArgment = count;

		//check command
		if(strcmp(args[0],"connect") == 0){
		
			if(cursoledArgment == 1 || cursoledArgment == 3){
				ret = rl_completion_matches(str,node_generator);
			}else if(cursoledArgment == 2 || cursoledArgment == 4){
				selectedNodeName = args[cursoledArgment - 1];
				ret = rl_completion_matches(str,pipe_generator);
			}
		}else if(strcmp(args[0],"disconnect") == 0){

			if(cursoledArgment == 1)
				ret = rl_completion_matches(str,node_generator);
			else if(cursoledArgment == 2){
				selectedNodeName = args[cursoledArgment - 1];
				ret = rl_completion_matches(str,pipe_generator);
			}
		}else if(strcmp(args[0],"const") == 0){

			if(cursoledArgment == 1)
				ret = rl_completion_matches(str,set_get_generator);
			if(cursoledArgment == 2)
				ret = rl_completion_matches(str,node_generator);
			else if(cursoledArgment == 3){
				selectedNodeName = args[cursoledArgment - 1];
				ret = rl_completion_matches(str,pipe_generator);
			}
		}

		//free
		free(cpyLine);
	}

	return ret;
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
	int code = nodeSystemAdd(argv[1],&argv[1]);
	if(code  != 0)
		fprintf(stdout,"add node failed:code %d\n",code);
	else
		fprintf(stdout,"add node success\n");
}

static void s_connect(int* argc,char* argv[]){
	if(*argc < 5){
		fprintf(stdout,"too few argment\n");
		return;
	}

	int code = nodeSystemConnect(argv[1],argv[2],argv[3],argv[4]);
	if(code != 0)
		fprintf(stdout,"pipe connect failed:code %d\n",code);
	else
		fprintf(stdout,"pipe connect success\n");
}

static void s_disconnect(int* argc,char* argv[]){
	if(*argc < 3){
		fprintf(stdout,"too few argment\n");
		return;
	}

	int code = nodeSystemDisConnect(argv[1],argv[2]);
	if(code != 0)
		fprintf(stdout,"pipe disconnect failed:code %d\n",code);
	else
		fprintf(stdout,"pipe disconnect success\n");
}

static void s_clear(int* argc,char* argv[]){
	system("clear");
}

static void s_list(int* argc,char* argv[]){
	nodeSystemList(argc,argv);
}

static void s_const(int* argc,char* argv[]){
	if(*argc < 2){
		fprintf(stdout,"too few argment\n");
		return;
	}

	if(strcmp(argv[1],"set") == 0){
		if(*argc < 5){
			fprintf(stdout,"too few argment\n");
			return;
		}

		int code = nodeSystemSetConst(argv[2],argv[3],*argc - 4,&argv[4]);

		if(code != 0)
			fprintf(stdout,"set const failed:code %d\n",code);
		else
			fprintf(stdout,"set const success\n");

	}else if(strcmp(argv[1],"get") == 0){
		if(*argc < 4){
			fprintf(stdout,"too few argment\n");
			return;
		}

		int code;
		char** value = nodeSystemGetConst(argv[2],argv[3],&code);

		if(code < 0)
			fprintf(stdout,"get const failed:code %d\n",code);
		else{
			fprintf(stdout,"get const success:\n");
			
			int i;
			for(i = 0;i < code;i++){
				fprintf(stdout,"\t%s[%d]:%s\n",argv[3],i,value[i]);
				free(value[i]);
			}
			free(value);
		}
	}else{

	}
}