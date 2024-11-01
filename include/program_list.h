#pragma once
#include <window-tree.h>
#include "defined.h"

typedef struct{
	int width;
	int height;
	int x;
	int y;
	uint8_t* cursor_status;
}programListData;

void ProgramList(char opcode,WINDOW_HANDLE handle
		,uint64_t oprand1,uint64_t oprand2,void* userData){

		programListData* data = userData;
		switch(opcode){
		case WINDOW_MESSAGE_CREATE:{
			break;
		}
		case WINDOW_MESSAGE_RESHAPE:{
			data->width  = (oprand1 >> 32)&0xFFFFFFFF;
			data->height = (oprand1		 )&0xFFFFFFFF;			
			break;
		}
		case WINDOW_MESSAGE_MOVE:{
			data->x = (oprand1 >> 32)&0xFFFFFFFF;
			data->y = (oprand1		)&0xFFFFFFFF;			
			break;
		}
		case WINDOW_MESSAGE_DISPLAY:{
			break;
		}
		case WINDOW_MESSAGE_KEYBOARD:{
			break;
		}
		case WINDOW_MESSAGE_MOUSE:{
			switch(oprand2&0xF){
				case 0:{						
					if(!(oprand2&0x100000000)){
						if((*data->cursor_status) == CURSOR_STATUS_VRESIZE_IDLE){
							(*data->cursor_status) = CURSOR_STATUS_VRESIZE_ACTIVE;
						}
					}else{
						(*data->cursor_status) = CURSOR_STATUS_IDLE;
					}
					break;
				}
				defalt:
			}
			break;
		}
		case WINDOW_MESSAGE_MOTION:{
			int x = (oprand1 >> 32)&0xFFFFFFFF;
			int y = (oprand1      )&0xFFFFFFFF;

			if((*data->cursor_status) == CURSOR_STATUS_VRESIZE_ACTIVE){
				wtResizeWindow(handle,data->width - x,data->height);
				wtMoveWindow(handle,data->x + x,0);
			}else{
				glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
				(*data->cursor_status) = CURSOR_STATUS_IDLE;
			}
			break;
		}
		case WINDOW_MESSAGE_PMOTION:{
			int x = (oprand1 >> 32)&0xFFFFFFFF;
			int y = (oprand1      )&0xFFFFFFFF;
			
			if(x <= resizeArea){
				glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
				(*data->cursor_status) = CURSOR_STATUS_VRESIZE_IDLE;
			}
			else{
				glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
				(*data->cursor_status) = CURSOR_STATUS_IDLE;
			}
			break;
		}
	}
}
