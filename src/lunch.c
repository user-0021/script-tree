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
	char* args[5];
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
	if(nodeSystemAdd(argv[1])  < 0){
		fprintf(stdout,"add node failed\n");
	}
}

static void s_connect(int* argc,char* argv[]){
}

static void s_clear(int* argc,char* argv[]){
	system("clear");
}
