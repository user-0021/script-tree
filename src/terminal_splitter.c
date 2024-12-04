#include <terminal_splitter.h>
#include <readline/readline.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

//global
const static uint16_t left_Min = 32;
static uint16_t left_width; 
static uint16_t left_rowCount = 0;

void ts_loop(){
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	left_width = w.ws_col * 0.7;
	if(left_width < left_Min)
		left_width = w.ws_col;

	if(!rl_point)
		left_rowCount = 0;
static int t = 77;
	uint16_t len = rl_point + 3 - left_rowCount * (left_width - 1);
	if(left_width == len){
		fprintf(stdout,"\e[1D\n\e[1C");
	}else if(len < 0){
		fprintf(stdout,"\e[1F\e[%dC",left_width);
	}
	else{
		if(t != len)
			fprintf(stdout,"|%d\n",len);
	}

	t = len;
}

void ts_print_left(const char* fmt,...){
}

void ts_print_right(const char* fmt,...){
}
