#pragma once
#include <stdint.h>
#include <FTGL/ftgl.h>

#define GLFW_MAJOR 3
#define GLFW_MINOR 2

#define BIT_SET(i) (1<<i)
#define BIT_CLEAR(i) (~(0<<i))

typedef void* WINDOW_HANDLE;

typedef struct{
} *WindowSystem;

typedef struct{
	int x;
	int y;
	int width;
	int height;
	int flag;
	uint32_t base_color;
	void* userData;
	WINDOW_HANDLE parent;
	int (*callback)(char,WINDOW_HANDLE,uint64_t,uint64_t,void*);
}Window;

#define WT_CURSOR_DEFAULT    (0)
#define WT_CURSOR_LEFT_RIGHT (1)

#define WINDOW_IGNORE_POSITION	(0x10)
#define WINDOW_IGNORE_SIZE		(0x20)

#define WINDOW_MESSAGE_CREATE	(0x00)
#define WINDOW_MESSAGE_DISPLAY	(0x01)
#define WINDOW_MESSAGE_RESHAPE	(0x02)
#define WINDOW_MESSAGE_KEYBOARD	(0x03)
#define WINDOW_MESSAGE_MOUSE	(0x04)
#define WINDOW_MESSAGE_MOTION	(0x05)
#define WINDOW_MESSAGE_PMOTION	(0x06)
#define WINDOW_MESSAGE_TIMER	(0x07)
#define WINDOW_MESSAGE_MOVE		(0x08)
#define WINDOW_MESSAGE_DESTROY	(0x09)

WindowSystem wtInit(int* argcp,char** argv);
WINDOW_HANDLE wtCreateWindow(Window* win,char* name);
int wtGetFlag(WINDOW_HANDLE handle);
void wtSetFlag(WINDOW_HANDLE handle,int flag);
void wtRegistTimer(WINDOW_HANDLE handle,unsigned int mills);
void wtDrawSquare(WINDOW_HANDLE handle,int x,int y,int width,int height);
void wtDrawCircle(WINDOW_HANDLE handle,int x,int y,int radius,int pieces);
void wtDrawText(WINDOW_HANDLE handle,int x,int y,char* str,FTGLfont* font);
void wtWindowSetTopWindow(WINDOW_HANDLE handle);
void wtMoveWindow(WINDOW_HANDLE handle,int x,int y);
void wtResizeWindow(WINDOW_HANDLE handle,int width,int height);
void wtSetColor4f(WINDOW_HANDLE handle,float r,float g,float b,float a);
void wtMainLoop();
void wtSetCursor(WINDOW_HANDLE handle,uint8_t mode);
