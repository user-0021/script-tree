#include <GL/glut.h>
#include <FTGL/ftgl.h>
#include <stdio.h>
#include <window-tree.h>
#include "program_list.h"
#include "node_viewer.h"
#include "defined.h"

#define FONT_PATH "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"

FTGLfont * baseFont;

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
	baseFont = ftglCreatePixmapFont(FONT_PATH);

	Window main;
	main.flag = WINDOW_RESOLUTION_x2 | WINDOW_IGNORE_POSITION | WINDOW_IGNORE_SIZE;
	main.base_color = 0x2F2F2FFF;
	main.userData = malloc(sizeof(scriptViewData));
	main.parent = NULL;
	main.callback = ScriptView;
	memset(main.userData,0,sizeof(scriptViewData));
	((scriptViewData*)main.userData)->font = baseFont;
	WINDOW_HANDLE handle = wtCreateWindow(&main,"script-tree");
	main.userData = malloc(sizeof(scriptViewData));
	((scriptViewData*)main.userData)->font = baseFont;
	WINDOW_HANDLE ahandle = wtCreateWindow(&main,"script-tree");
	
	wtMainLoop();
	return 0;
}

