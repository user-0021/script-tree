#include "window-tree.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <linear_list.h>

#define CHAR_TO_F(c) ((float)(0xFF & (unsigned int)c))
#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
typedef struct{
	float r;
	float g;
	float b;
	float a;
} _wt_RGBA;

typedef struct _window_class{
	GLFWwindow* window;
	_wt_RGBA backColor;
	char* windowName;
	float vscale;
	float hscale;
	int left;
	int right;
	int top;
	int bottom;
	struct _window_class* current;

	Window body;
	struct _window_class** children;
}WindowClass;


typedef struct{
	WindowClass** windowList;
}WindowSystemClass;

WindowSystemClass* wtSystem = NULL;
WindowClass* timerList[10] = {};

void glDisplay(void);
void glReShape(int w,int h);
void glKeyboard(unsigned char key, int x, int y);
void glMouse(int button, int state, int x ,int y);
void glMotion(int x, int y);
void glPassiveMotion(int x, int y);
void glClose(void);
void glTimer(int value);
void applyFlag(WindowClass* class);
void calcGrobalPos(WindowClass* class);
WindowClass* getPointOwner(WindowClass* class,int x,int y);

//init window system
WindowSystem wtInit(int* argcp,char** argv){//init env data
	WindowSystemClass* class = malloc(sizeof(WindowSystemClass));
	memset(class,0,sizeof(WindowSystemClass));

	//init library
    if (!glfwInit()) {
        fprintf(stderr,"Failed to initialize GLFW\n");
		exit(0);
    }										

	//seet version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFW_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_MINER);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	//init list
	class->windowList = LINEAR_LIST_CREATE(WindowClass*);
	wtSystem = class;

	return (WindowSystem)class;
}

//create Window
WINDOW_HANDLE wtCreateWindow(Window* win,char* name){
	//create object
	WindowClass* class = malloc(sizeof(WindowClass));
	if(!class){
		perror("")
	}

	memset(class,0,sizeof(WindowClass));

	//copy data
	memcpy(&class->body,win,sizeof(Window));
	class->windowName = malloc(strlen(name)+1);
	strcpy(class->windowName,name);

	//init list
	class->children = LINEAR_LIST_CREATE(WindowClass*);
	if(win->parent){
		//calc scale
		WindowClass* parent = class->body.parent;
		class->hscale = parent->hscale * 
							((float)parent->body.width/(float)class->body.width);
		class->vscale = parent->vscale *
		   					((float)parent->body.height/(float)class->body.height);

		//calc global pos
		class->left   = parent->left   + class->body.x;
		class->bottom = parent->bottom + class->body.y;
		class->right  = class->left    + class->body.width;
		class->top    = class->bottom  + class->body.height;

		//check limit
		class->right = MIN(class->right,parent->right);
		class->top   = MIN(class->top,parent->top);

		//create Vartiul Window
		LINEAR_LIST_PUSH(((WindowClass*)class->body.parent)->children,class);
	}else{	


		//create ftgl window
		class->window = glfwCreateWindow(class->width,class->height,name, NULL, NULL);
    	if (!class->window) {
        	printf("Failed to create window 1\n");
        	glfwTerminate();
        	return -1;
    	}
		
		//init pos

		//setColor
		char* color = (void*)&class->body.base_color;
		glClearColor(CHAR_TO_F(color[3])*(1.0f/255.0f),CHAR_TO_F(color[2])*(1.0f/255.0f)
					,CHAR_TO_F(color[1])*(1.0f/255.0f),CHAR_TO_F(color[0])*(1.0f/255.0f));

		//apply flag
		applyFlag(class);
		
		//calc scale
		if(class->body.flag & 1)
			class->hscale = class->vscale = 
					1.0f / ((unsigned int)1<<((class->body.flag>>1)&0x03));
		else
			class->hscale = class->vscale =
	 		  		(float)((unsigned int)1<<((class->body.flag>>1)&0x03));

		//set global pos
		class->right = class->body.width;
		class->top  = class->body.height;
		
		//regist callback
		glutDisplayFunc(glDisplay);
		glutReshapeFunc(glReShape);
		glutKeyboardFunc(glKeyboard);
		glutMouseFunc(glMouse);
		glutMotionFunc(glMotion);
		glutPassiveMotionFunc(glPassiveMotion);
		glutCloseFunc(glClose);
	}	

	//push list
	LINEAR_LIST_PUSH(wtSystem->windowList,class);
	
	//call
	class->body.callback(WINDOW_MESSAGE_CREATE,class,0,0,class->body.userData);

	return (WINDOW_HANDLE)class;
}

//setFlag
void wtSetFlag(WINDOW_HANDLE handle,int flag){
	((WindowClass*)handle)->body.flag = flag;
}

//getFlag
int wtGetFlag(WINDOW_HANDLE handle){
	return ((WindowClass*)handle)->body.flag;
}

void wtRegistTimer(WINDOW_HANDLE handle,unsigned int mills){
	int i;
	int max = (sizeof(timerList) / sizeof(WindowClass*));


	for(i = 0;i < max;i++){
		if(timerList[i] == NULL){
			timerList[i] = (WindowClass*)handle;
			glutTimerFunc(mills,glTimer,i);
		}
	}
}

void wtDrawSquare(WINDOW_HANDLE handle,int x,int y,int width,int height){
	WindowClass *class = handle;
	WindowClass *parent = (WindowClass*)class->body.parent;

	if(parent){
		if(class->left <= parent->left)
			x += class->left - parent->left;
		if(class->bottom <= parent->bottom)
			y += class->bottom - parent->bottom;
	}

	width  = (x+width)  * class->hscale;
	height = (y+height) * class->vscale;
	x *= class->hscale;
	y *= class->vscale;

	int points[4][2] = {{x,y},{width,y},{width,height},{x,height}};
	unsigned char index[4] = {0,1,3,2};
	
	glVertexPointer(2,GL_INT,0,points);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_BYTE,index);
}

void wtDrawCircle(WINDOW_HANDLE handle,int x,int y,int radius,int pieces){
	WindowClass *class = handle;
	WindowClass *parent = (WindowClass*)class->body.parent;

	if(parent){
		if(class->left <= parent->left)
			x += class->left - parent->left;
		if(class->bottom <= parent->bottom)
			y += class->bottom - parent->bottom;
	}

	x *= class->hscale;
	y *= class->vscale;
	
	#if INT_MAX == 0x7FFFFFFF
		int* points = malloc((pieces + 1) << 3);
		unsigned int* index = malloc((pieces + 2) <<2);
	#elif INT_MAX == 0x7FFFFFFFFFFFFFFF
		int* points = malloc((pieces + 1) << 4);
		unsigned int* index = malloc((pieces + 2) <<3);
	#else
		"ERR:This program only support 4byte int or 8byte int"
	#endif
	
	//center point
	points[0] = x;
	points[1] = y;
	index[0] = 0;

	int i;
	float rad = 0;
	float hscaleRadius = radius * class->hscale;
	float vscaleRadius = radius * class->vscale;
	float deltaRad = (2*M_PI)/pieces;
	for(i = 1;i <= pieces;i++){
		points[(i<<1)  ] = x + cos(rad)*hscaleRadius;
		points[(i<<1)+1] = y + sin(rad)*vscaleRadius;

		rad += deltaRad;
		index[i] = i;
	}
	index[pieces+1] = 1;

	glVertexPointer(2,GL_INT,0,points);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawElements(GL_TRIANGLE_FAN,pieces+2,GL_UNSIGNED_INT,index);

	free(points);
	free(index);
}

/*
void wtDrawText(WINDOW_HANDLE handle,int x,int y,char* str,FTGLfont* font){
	WindowClass *class = handle;
	WindowClass *parent = (WindowClass*)class->body.parent;

	if(parent){
		if(class->left <= parent->left)
			x += class->left - parent->left;
		if(class->bottom <= parent->bottom)
			y += class->bottom - parent->bottom;
	}
	
	x *= class->hscale;
	y *= class->vscale;

	glRasterPos2i(x,y);
	ftglRenderFont(font, str, FTGL_RENDER_ALL);
}
*/

void wtWindowSetTopWindow(WINDOW_HANDLE handle){
	WindowClass *class = handle;

	WindowClass** list;
	WindowClass** itr;

	if(class->body.parent)
		list = ((WindowClass*)class->body.parent)->children;
	else
		list = wtSystem->windowList;

	LINEAR_LIST_FOREACH(list,itr){
		if((*itr) == class){
			WindowClass** next = LINEAR_LIST_NEXT(itr);
			WindowClass** prev = LINEAR_LIST_PREV(itr);
			WindowClass** last = LINEAR_LIST_LAST(itr);

			WindowClass** next_prev = LINEAR_LIST_PREV(next);
			WindowClass** prev_next = LINEAR_LIST_NEXT(prev);
			next_prev = prev;
			prev_next = next;

			WindowClass** last_next = LINEAR_LIST_NEXT(last);
			WindowClass** itr_prev  = LINEAR_LIST_PREV(prev);
			last_next = itr;
			itr_prev  = last;

			return;
		}
	}
}

void wtMoveWindow(WINDOW_HANDLE handle,int x,int y){
	WindowClass *class = handle;

	class->body.x  = x;
	class->body.y  = y;

	if(class->body.parent == NULL)
		glutPositionWindow(x,y);

	class->body.callback(WINDOW_MESSAGE_MOVE,class,
		(((uint64_t)x<<32))|(y&0xFFFFFFFF),0,class->body.userData);

	calcGrobalPos(class);
}

void wtResizeWindow(WINDOW_HANDLE handle,int width,int height){
	WindowClass *class = handle;

	class->body.width  = width;
	class->body.height = height;

	if(class->body.parent == NULL){
		glutReshapeWindow(width,height);

		//setData
		class->right = width;
		class->top   = height;
	}else{
		class->body.callback(WINDOW_MESSAGE_RESHAPE,class,
		(((uint64_t)width<<32))|(height&0xFFFFFFFF),0,class->body.userData);
	}

	calcGrobalPos(class);
}

void wtReflesh(){
	glutPostRedisplay();
}

void wtMainLoop(){
	while(1){
		glutMainLoopEvent();
	}
}

//Child window display
void callChildrenDisplay(WindowClass* class){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(class->children,itr){
		//check pos
		if((*itr)->left >= class->right || (*itr)->bottom >= class->top
				|| (*itr)->right <= class->left || (*itr)->top <= class->bottom)
			return;

		//init view port
		glViewport(MAX((*itr)->left,class->left),MAX((*itr)->bottom,class->bottom)
					,(*itr)->right - (*itr)->left,(*itr)->top - (*itr)->bottom);
		
		//set Color
		char* color = (void*)&(*itr)->body.base_color;
		glColor4f(CHAR_TO_F(color[3])*(1.0f/255.0f),CHAR_TO_F(color[2])*(1.0f/255.0f)
					,CHAR_TO_F(color[1])*(1.0f/255.0f),CHAR_TO_F(color[0])*(1.0f/255.0f));
		


		//fill base
		if(color[0])
			wtDrawSquare(*itr,0,0,(*itr)->body.width,(*itr)->body.height);		

		//callback
		(*itr)->body.callback(WINDOW_MESSAGE_DISPLAY,*itr,0,0,(*itr)->body.userData);
		
		callChildrenDisplay(*itr);
	}
}

void glDisplay(void){ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
	int id = glutGetWindow();
	
	//clear
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(id == (*itr)->windowID){//match id

			//init viewport
			glViewport(0,0,(*itr)->body.width,(*itr)->body.height);
			//callback
			(*itr)->body.callback(WINDOW_MESSAGE_DISPLAY,*itr,0,0,(*itr)->body.userData);
			//children callback
			callChildrenDisplay(*itr);
			//resrt viewport
			glViewport(0,0,(*itr)->body.width,(*itr)->body.height);
			break;
		}
	}
	
	glutSwapBuffers();
}

//Child window reshape
void calcChildrenGlobalPos(WindowClass* class){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(class->children,itr){
		//calc scale
		(*itr)->hscale = class->hscale * 
							((float)class->body.width/(float)(*itr)->body.width);
		(*itr)->vscale = class->vscale *
		 					((float)class->body.height/(float)(*itr)->body.height);			
		
		//calc global pos
		(*itr)->left   = class->left   + (*itr)->body.x;
		(*itr)->bottom = class->bottom + (*itr)->body.y;
		(*itr)->right  = (*itr)->left    + (*itr)->body.width;
		(*itr)->top    = (*itr)->bottom  + (*itr)->body.height;
		
		//check limit
		(*itr)->top	   = MIN((*itr)->top   ,class->top   );
		(*itr)->right  = MIN((*itr)->right ,class->right );

		calcChildrenGlobalPos(*itr);
	}

}

void glReShape(int w,int h){
	int id = glutGetWindow();

	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(id == (*itr)->windowID){
			//setData
			(*itr)->right = (*itr)->body.width = w;
			(*itr)->top = (*itr)->body.height = h;
			
			//calc scale
			if((*itr)->body.flag & 1)
				(*itr)->hscale = (*itr)->vscale = 
						1.0f / ((unsigned int)1<<(((*itr)->body.flag>>1)&0x03));
			else
				(*itr)->hscale = (*itr)->vscale =
				   		(float)((unsigned int)1<<(((*itr)->body.flag>>1)&0x03));

			//calc buffer size
			int pWidth,pHeight;
			if((*itr)->body.flag & 1){
				pWidth  = w  >> (((*itr)->body.flag>>1) & 0x03);
				pHeight = h  >> (((*itr)->body.flag>>1) & 0x03);
			}else{
				pWidth  = w  << (((*itr)->body.flag>>1) & 0x03);
				pHeight = h  << (((*itr)->body.flag>>1) & 0x03);
			}

			//children
			calcChildrenGlobalPos(*itr);
		
			//init pos and size
			glLoadIdentity(); 	
			gluOrtho2D(0,pWidth,0,pHeight);
			glViewport(0,0,w,h);
			
			(*itr)->body.callback(WINDOW_MESSAGE_RESHAPE,*itr,
					(((uint64_t)w<<32))|(h&0xFFFFFFFF),0,(*itr)->body.userData);
		}
	}
}

void glKeyboard(unsigned char key, int x, int y){
	int id = glutGetWindow();
	
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(id == (*itr)->windowID){
			y = (*itr)->body.height - y;
			
			WindowClass* owner = getPointOwner((*itr),x,y);
				
			x = (x - owner->left);		
			y = (y - owner->bottom);

			owner->body.callback(WINDOW_MESSAGE_KEYBOARD,owner,
					(((uint64_t)x<<32))|(y&0xFFFFFFFF),key,owner->body.userData);
		}
	}
}

void glMouse(int button, int state, int x ,int y){	
	int id = glutGetWindow();
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(id == (*itr)->windowID){
			y = (*itr)->body.height - y;
	
			WindowClass* owner = getPointOwner((*itr),x,y);
				
			x = (x - owner->left);		
			y = (y - owner->bottom);

			owner->body.callback(WINDOW_MESSAGE_MOUSE,owner,
					(((uint64_t)x<<32))|(y&0xFFFFFFFF)
					,(((uint64_t)state<<32))|(button&0xFFFFFFFF),owner->body.userData);
		}
	}
}

void glMotion(int x, int y){
	int id = glutGetWindow();
	
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(id == (*itr)->windowID){
			y = (*itr)->body.height - y;

			WindowClass* owner = getPointOwner((*itr),x,y);
				
			x = (x - owner->left);		
			y = (y - owner->bottom);

			owner->body.callback(WINDOW_MESSAGE_MOTION,owner,
					(((uint64_t)x<<32))|(y&0xFFFFFFFF),0,owner->body.userData);
		}
	}
}

void glPassiveMotion(int x, int y){
	int id = glutGetWindow();
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(id == (*itr)->windowID){
			y = (*itr)->body.height - y;
			
			WindowClass* owner = getPointOwner((*itr),x,y);
				
			x = (x - owner->left);		
			y = (y - owner->bottom);			

			owner->body.callback(WINDOW_MESSAGE_PMOTION,owner,
					(((uint64_t)x<<32))|(y&0xFFFFFFFF),0,owner->body.userData);
		}
	}
}

void wtDeleatewindowAll(WindowClass* class){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(class->children,itr){
		wtDeleatewindowAll(*itr);
	}

	LINEAR_LIST_RELEASE(class->children);
	free(class);
}

void glClose(void){
	int id = glutGetWindow();
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(id == (*itr)->windowID){
			int ret = (*itr)->body.callback(WINDOW_MESSAGE_DESTROY,*itr,0,0,(*itr)->body.userData);
			if(!ret){
				glutDestroyWindow(id);
				return;
				wtDeleatewindowAll(*itr);
				WindowClass** next = LINEAR_LIST_NEXT(itr);
				WindowClass** prev = LINEAR_LIST_NEXT(itr);
				if(next){
					WindowClass** next_prev = LINEAR_LIST_PREV(next);
					next_prev = prev;
				}
				WindowClass** prev_next = LINEAR_LIST_NEXT(prev);
				prev_next = next;

				free(itr);
			}
		}
	}
}

void glTimer(int value){
	timerList[value]->body.callback(WINDOW_MESSAGE_TIMER,timerList[value],
			0,0,timerList[value]->body.userData);
	
	timerList[value] = NULL;
}

void applyFlag(WindowClass* class){
	//calc buffer size
	int pWidth,pHeight;
	if(class->body.flag & 1){
		pWidth  = class->body.width  >> ((class->body.flag>>1) & 0x03);
		pHeight = class->body.height >> ((class->body.flag>>1) & 0x03);
	}else{
		pWidth  = class->body.width  << ((class->body.flag>>1) & 0x03);
		pHeight = class->body.height << ((class->body.flag>>1) & 0x03);
	}
	
	//init pos and size
	gluOrtho2D(0,pWidth,0,pHeight);
	glViewport(0,0,class->body.width,class->body.height);
}

void calcGrobalPos(WindowClass* class){
	
	if(class->body.parent == NULL){
		//calc scale
		if(class->body.flag & 1)
			class->hscale = class->vscale = 
					1.0f / ((unsigned int)1<<((class->body.flag>>1)&0x03));
		else
			class->hscale = class->vscale =
			   		(float)((unsigned int)1<<((class->body.flag>>1)&0x03));

		calcChildrenGlobalPos(class);
	}else{
		calcChildrenGlobalPos(class->body.parent);
	}
	
}


WindowClass* getPointOwner(WindowClass* class,int x,int y){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(class->children,itr){

		if((*itr)->left <= x && (*itr)->bottom <= y 
			&& (*itr)->right >= x && (*itr)->top >= y){
			return getPointOwner((*itr),x,y);
		}	
	}
	return class;
}
