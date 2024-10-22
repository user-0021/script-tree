#include <GL/glut.h>
#include <stdio.h>
#include <window-tree.h>
#include <FTGL/ftgl.h>

FTGLfont * baseFont;

void allback(char opcode,WINDOW_HANDLE handle
		,uint64_t oprand1,uint64_t oprand2,void* userData){
	switch(opcode){
		case WINDOW_MESSAGE_CREATE:{
			break;
		}
		case WINDOW_MESSAGE_RESHAPE:{
			break;
		}
		case WINDOW_MESSAGE_DISPLAY:{
			glColor4f(0,0,0,1.0);
			wtDrawText(handle,0,0,"てすとtest",baseFont);
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
	}
}

void Callback(char opcode,WINDOW_HANDLE handle
		,uint64_t oprand1,uint64_t oprand2,void* userData){
	switch(opcode){
		case WINDOW_MESSAGE_CREATE:{
			printf("create\n");
			
			Window main;
			main.x = 50;
			main.y = 50;
			main.width = 100;
			main.height = 100;
			main.base_color = 0xFF0000FF;
			main.userData = NULL;
			main.parent = handle;
			main.callback = allback;
	
			wtCreateWindow(&main,NULL);
			
			break;
		}
		case WINDOW_MESSAGE_RESHAPE:{
			printf("reshape %d %d\n"
					,(int)(oprand1>>32)&0xFFFFFFFF,(int)oprand1&0xFFFFFFFF);
			break;
		}
		case WINDOW_MESSAGE_DISPLAY:{
			printf("display\n");
			glColor4f(0.2,1.0,0.1,1.0);
			wtDrawCircle(handle,150,150,100,5);
			glColor4f(0,0,0,1.0);
			wtDrawText(handle,50,200,"てすとtest",baseFont);
			break;
		}
		case WINDOW_MESSAGE_KEYBOARD:{
			printf("key\n");
			break;
		}
		case WINDOW_MESSAGE_MOUSE:{
			printf("mouse\n");
			break;
		}
		case WINDOW_MESSAGE_MOTION:{
			printf("motion\n");
			break;
		}
		case WINDOW_MESSAGE_PMOTION:{
			printf("passive motion\n");
			break;
		}
	}
}

int main(int argc,char* argv[])
{
	//glutInit(&argc,argv);
	//glutInitDisplayMode(GLUT_RGBA);
	//glutCreateWindow(argv[0]);
	//glutDisplayFunc(display);
 	//gluOrtho2D(0, 200, 0, 200);
	
	wtInit(&argc,argv);
	baseFont = ftglCreatePixmapFont("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc");

	Window main;
	main.x = 50;
	main.y = 50;
	main.width = 400;
	main.height = 400;
	main.flag = WINDOW_RESOLUTION_x2;
	main.base_color = 0x2F2F2FFF;
	main.userData = NULL;
	main.parent = NULL;
	main.callback = Callback;

	WINDOW_HANDLE handle = wtCreateWindow(&main,"test");
	
	glutMainLoop();
	return 0;
}

