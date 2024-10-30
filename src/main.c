#include <GL/glut.h>
#include <stdio.h>
#include <window-tree.h>
#include <FTGL/ftgl.h>

#define FONT_PATH "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"

typedef struct{
	float scale;
	int32_t width;
	int32_t height;
	uint32_t x;
	uint32_t y;
}mainWindowData;

FTGLfont * baseFont;

void MainWindow(char opcode,WINDOW_HANDLE handle
		,uint64_t oprand1,uint64_t oprand2,void* userData){
	const static int period = 40;
	const static int radix  = 3;
	const static int pieces = 40;
	const static int pieces_max = 40;
	const static unsigned int mills = 100;

	mainWindowData* data = userData;
	switch(opcode){
		case WINDOW_MESSAGE_CREATE:{
			data->scale = 1;
			data->x = 0;
			data->y = 0;
			wtRegistTimer(handle,mills);
			break;
		}
		case WINDOW_MESSAGE_RESHAPE:{
			data->width  = (oprand1 >> 32)&0xFFFFFFFF;
			data->height = (oprand1		)&0xFFFFFFFF;
			break;
		}
		case WINDOW_MESSAGE_DISPLAY:{
			//display back ground
			glColor4f(0.5,0.5,0.5,1.0);
	
			unsigned int period_scale = period * data->scale;
	
			if(!period_scale)
				period_scale = 1;
			
			unsigned int dotRow    = data->width  / period_scale;
			unsigned int dotColumn = data->height / period_scale;		
			int x = period_scale;
			int* y = malloc(sizeof(int) * dotColumn);		
			int r = radix * data->scale;
			int p = pieces * data->scale;
			y[0] = period_scale;

			if(p > pieces_max)
				p = pieces_max;

			if(r){
				unsigned int i;
				for(i = 0;i < dotRow;i++){
					unsigned int j;
					for(j = 0;j < dotColumn;j++){
						if(!i && j)
							y[j] = y[j-1] + period_scale;

						wtDrawCircle(handle,x,y[j],r,p);
					}

					x += period_scale;
				}
			}
			
			//write scale
			glColor4f(0,0,0,1.0);
			char percent[5];
			sprintf(percent,"%3.0f",data->scale*100);
			wtDrawText(handle,0,0,percent,baseFont);
			break;
		}
		case WINDOW_MESSAGE_KEYBOARD:{
			break;
		}
		case WINDOW_MESSAGE_MOUSE:{
			switch(oprand2&0xF){
				case 3:{
					if(oprand2&0xF00000000)
						if(data->scale > 0.11)
							data->scale -= 0.1;
					break;
				}
				case 4:{
					if(oprand2&0xF00000000)
						if(data->scale < 9.99)
							data->scale += 0.1;
					break;
				}
				default:
			}

			break;
		}
		case WINDOW_MESSAGE_MOTION:{
			break;
		}
		case WINDOW_MESSAGE_PMOTION:{
			break;
		}
		case WINDOW_MESSAGE_TIMER:{
			wtRegistTimer(handle,mills);
			wtReflesh();
			break;
		}
		default:
	}
}

int main(int argc,char* argv[])
{
	wtInit(&argc,argv);
	baseFont = ftglCreatePixmapFont(FONT_PATH);

	Window main;
	main.flag = WINDOW_RESOLUTION_x2 | WINDOW_IGNORE_POSITION | WINDOW_IGNORE_SIZE;
	main.base_color = 0x2F2F2FFF;
	main.userData = malloc(sizeof(mainWindowData));
	main.parent = NULL;
	main.callback = MainWindow;

	WINDOW_HANDLE handle = wtCreateWindow(&main,"script-tree");
	
	glutMainLoop();
	return 0;
}

