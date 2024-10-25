#include <GL/glut.h>
#include <stdio.h>
#include <window-tree.h>
#include <FTGL/ftgl.h>

#define FONT_PATH "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"

FTGLfont * baseFont;

void MainWindow(char opcode,WINDOW_HANDLE handle
		,uint64_t oprand1,uint64_t oprand2,void* userData){
	//userData* | scale | width | height |
	//			| float | int32 | int32  |
	const static int period = 40;
	const static int radix  = 3;
	const static int pieces = 40;
	const static int pieces_max = 40;
	const static unsigned int mills = 100;

	switch(opcode){
		case WINDOW_MESSAGE_CREATE:{
			((float*)userData)[0] = 1;
			wtRegistTimer(handle,mills);
			break;
		}
		case WINDOW_MESSAGE_RESHAPE:{
			int32_t* vec = (int32_t*)(((float*)userData)+1);
			vec[0] = (oprand1 >> 32)&0xFFFFFFFF;
			vec[1] = (oprand1		)&0xFFFFFFFF;
			break;
		}
		case WINDOW_MESSAGE_DISPLAY:{
			//display back ground
			glColor4f(0.5,0.5,0.5,1.0);
	
			float scale = ((float*)userData)[0];
			unsigned int period_scale = period * scale;
	
			if(!period_scale)
				period_scale = 1;
			
			int32_t* vec = (int32_t*)(((float*)userData)+1);
			unsigned int dotRow    = vec[0] / period_scale;
			unsigned int dotColumn = vec[1] / period_scale;		
			int x = period_scale;
			int* y = malloc(sizeof(int) * dotColumn);		
			int r = radix * scale;
			int p = pieces * scale;
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
			sprintf(percent,"%3.0f",scale*100);
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
						if(((float*)userData)[0] > 0.11)
							((float*)userData)[0] -= 0.1;
					break;
				}
				case 4:{
					if(oprand2&0xF00000000)
						if(((float*)userData)[0] < 9.99)
							((float*)userData)[0] += 0.1;
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
	main.userData = malloc(sizeof(float) + sizeof(int32_t)*2);
	main.parent = NULL;
	main.callback = MainWindow;

	WINDOW_HANDLE handle = wtCreateWindow(&main,"script-tree");
	
	glutMainLoop();
	return 0;
}

