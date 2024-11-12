#include <stdio.h>
#include <window-tree.h>
#include "program_list.h"
#include "node_viewer.h"
#include "defined.h"


int test(char opcode,WINDOW_HANDLE handle
		,uint64_t oprand1,uint64_t oprand2,void* userData){
	
	switch(opcode){
		case WINDOW_MESSAGE_CREATE:{
			break;
		}
		case WINDOW_MESSAGE_RESHAPE:{
			break;
		}
		case WINDOW_MESSAGE_DISPLAY:{
			break;
		}
		case WINDOW_MESSAGE_KEYBOARD:{
			break;
		}
		case WINDOW_MESSAGE_MOUSE:{
			break;
		}
		case WINDOW_MESSAGE_MOTION:{
			break;
		}
		case WINDOW_MESSAGE_PMOTION:{
			break;
		}
		case WINDOW_MESSAGE_TIMER:{
			break;
		}
		default:
	}

	return 0;
}

void aglDisplay(void){ 
}

int main(int argc,char* argv[])
{
	wtInit(&argc,argv);

	Window main;
	main.flag =  WINDOW_IGNORE_POSITION | WINDOW_IGNORE_SIZE;
	main.base_color = 0x2F2F2FFF;
	main.userData = malloc(sizeof(nodeViewData));
	main.parent = NULL;
	main.callback = NodeView;
	memset(main.userData,0,sizeof(nodeViewData));
	WINDOW_HANDLE handle = wtCreateWindow(&main,"script-tree");
	main.userData = malloc(sizeof(nodeViewData));
	memset(main.userData,0,sizeof(nodeViewData));
	WINDOW_HANDLE ahandle = wtCreateWindow(&main,"script-tree");
	
	wtMainLoop();
	return 0;
}
