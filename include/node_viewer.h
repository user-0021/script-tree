#pragma once
#include <window-tree.h>
#include "defined.h"

typedef struct{
	FTGLfont* font;
	float scale;
	int width;
	int height;
	int x;
	int y;
	uint8_t cursor_status;

	int progWidth;
	int progHeight;
	WINDOW_HANDLE programList;
}scriptViewData;


void DrawBackGround(WINDOW_HANDLE handle, scriptViewData* data);

int ScriptView(char opcode,WINDOW_HANDLE handle
		,uint64_t oprand1,uint64_t oprand2,void* userData){
	
	scriptViewData* data = userData;
	switch(opcode){
		case WINDOW_MESSAGE_CREATE:{
			data->scale = 1;
			wtRegistTimer(handle,mills);

			Window prog = {};
			prog.base_color = 0x6F6F6FFF;
			prog.parent = handle;
			prog.userData = malloc(sizeof(programListData));
			memset(prog.userData,0,sizeof(programListData));
			((programListData*)prog.userData)->cursor_status = &data->cursor_status;
			((programListData*)prog.userData)->width  = &data->progWidth;
			((programListData*)prog.userData)->height = &data->progHeight;
			prog.callback = ProgramList;
			data->programList = wtCreateWindow(&prog,"a");
			break;
		}
		case WINDOW_MESSAGE_RESHAPE:{
			float progScale = data->progWidth / (float)data->width;

			data->width  = (oprand1 >> 32)&0xFFFFFFFF;
			data->height = (oprand1		)&0xFFFFFFFF;
			
			data->progHeight = data->height;
			
			if(data->width <= data->progWidth)
				data->progWidth = data->width * progScale;
			else if(!data->progWidth )
				data->progWidth = 0.2 * data->width;
			
			wtResizeWindow(data->programList,data->progWidth,data->progHeight);
			wtMoveWindow(data->programList,data->width - data->progWidth,0);
			break;
		}
		case WINDOW_MESSAGE_DISPLAY:{
			//display back ground
			DrawBackGround(handle,data);
			
			//write scale
			glColor4f(0,0,0,1.0);
			char percent[5];
			sprintf(percent,"%3.0f",data->scale*100);
			wtDrawText(handle,0,0,percent,data->font);

			break;
		}
		case WINDOW_MESSAGE_KEYBOARD:{
			break;
		}
		case WINDOW_MESSAGE_MOUSE:{
			switch(oprand2&0xF){
				case 0:{						
					if(!(oprand2&0x100000000)){
						if(data->cursor_status == CURSOR_STATUS_VRESIZE_IDLE){
							data->cursor_status = CURSOR_STATUS_VRESIZE_ACTIVE;
						}
					}else if(data->cursor_status != CURSOR_STATUS_IDLE){
						glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
						data->cursor_status = CURSOR_STATUS_IDLE;
					}
					break;
				}
				case 3:{
					if(!(oprand2&0x1000000000))
						if(data->scale > 0.11){/*
							int x = ((oprand1 >> 32)&0xFFFFFFFF) - (data->width  >> 1);
							int y = ((oprand1      )&0xFFFFFFFF) - (data->height >> 1);
							float moveScale = zoomScale / (data->scale*data->scale); 
							data->x += x * moveScale;
							data->y += y * moveScale;*/
							
							data->scale -= zoomScale;
						}
					break;
				}
				case 4:{
					if(!(oprand2&0x100000000))
						if(data->scale < 9.99){/*
							int x = ((oprand1 >> 32)&0xFFFFFFFF) - (data->width  >> 1);
							int y = ((oprand1      )&0xFFFFFFFF) - (data->height >> 1);
							float moveScale = zoomScale / (data->scale*data->scale); 
							data->x -= x * moveScale;
							data->y -= y * moveScale;*/
							
							data->scale += zoomScale;
						}
					break;
				}
				default:
			}

			break;
		}
		case WINDOW_MESSAGE_MOTION:{
			int x = (oprand1 >> 32)&0xFFFFFFFF;
			int y = (oprand1      )&0xFFFFFFFF;

			if(data->cursor_status  == CURSOR_STATUS_VRESIZE_ACTIVE){//programListResize
				data->progHeight = data->height;
				data->progWidth = data->width - x;
				
				wtResizeWindow(data->programList,data->progWidth,data->progHeight);
				wtMoveWindow(data->programList,data->width - data->progWidth,0);
			}else if(data->cursor_status != CURSOR_STATUS_IDLE){//default
				glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
				data->cursor_status = CURSOR_STATUS_IDLE;
			}
			break;
		}
		case WINDOW_MESSAGE_PMOTION:{
			int x = (oprand1 >> 32)&0xFFFFFFFF;
			int y = (oprand1      )&0xFFFFFFFF;
			
			int limit = data->width - data->progWidth;
			if(x >= (limit - resizeArea)){//programListResize
				glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
				data->cursor_status = CURSOR_STATUS_VRESIZE_IDLE;
			}
			else if(data->cursor_status != CURSOR_STATUS_IDLE){//default
				glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
				data->cursor_status = CURSOR_STATUS_IDLE;
			}
			break;
		}
		case WINDOW_MESSAGE_TIMER:{
			wtRegistTimer(handle,mills);
			wtReflesh();
			break;
		}
		default:
	}
	return 0;
}


void DrawBackGround(WINDOW_HANDLE handle, scriptViewData* data){
	glColor4f(0.5,0.5,0.5,1.0f);
	
	unsigned int period_scale = period * data->scale;
	
	if(!period_scale)
		period_scale = 1;
			
	unsigned int dotRow    = (data->width  / period_scale) + 1;
	unsigned int dotColumn = (data->height / period_scale) + 1;		
			
	int x = ((int)(data->x*data->scale) + (data->width>>1)) % period_scale;
	int* y = malloc(sizeof(int) * dotColumn);		
	y[0] = ((int)(data->y*data->scale) + (data->height>>1)) % period_scale;
			
	int r = radix * data->scale;
	int p = pieces * data->scale;

	if(p > pieces_max)
		p = pieces_max;

	if(r){
		unsigned int i;
		for(i = 0;i < dotRow;i++){
			unsigned int j;
			for(j = 0;j < dotColumn;j++){
				if(!i && j)
					y[j] = y[j-1] + period_scale;
				
				if(!(x & 0x80000000 || y[j] & 0x80000000))
					wtDrawCircle(handle,x,y[j],r,p);
			}

			x += period_scale;
		}
	}
}
