#include "window-tree.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <linear_list.h>
#include "shader.h"
#include "font.h"

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
	FT_Library fontLibrary;
	unsigned int shaderProgram;
	unsigned int textShaderProgram;
	struct _window_class** children;
}WindowClass;


typedef struct{
	WindowClass** windowList;
}WindowSystemClass;

//global
WindowSystemClass* wtSystem = NULL;
WindowClass* timerList[10] = {};


//callback
static void _glfw_callback_windowposfun(GLFWwindow *window, int xpos, int ypos);
static void _glfw_callback_windowsizefun(GLFWwindow *window, int width, int height);
static void _glfw_callback_windowrefreshfun(GLFWwindow *window);
static void _glfw_callback_windowfocusfun(GLFWwindow *window, int focused);
static void _glfw_callback_windowiconifyfun(GLFWwindow *window, int iconified);
static void _glfw_callback_windowmaximizefun(GLFWwindow *window, int maximized);
static void _glfw_callback_framebuffersizefun(GLFWwindow *window, int width, int height);
static void _glfw_callback_windowcontentscalefun(GLFWwindow *window, float xscale, float yscale);
static void _glfw_callback_cursorposition(GLFWwindow* window, double xpos, double ypos);

//func
static void wtWindowLoop(WindowClass** itr);
static void callChildrenDisplay(WindowClass* class);
static void calcGrobalPos(WindowClass* class);
static WindowClass* getPointOwner(WindowClass* class,int x,int y);
static void wtDeleateWindowAll(WindowClass* class);


//init window system
WindowSystem wtInit(int* argcp,char** argv){//init env data
	WindowSystemClass* class = malloc(sizeof(WindowSystemClass));
	if(!class){
		perror(__func__);
		exit(-1);
	}
	memset(class,0,sizeof(WindowSystemClass));

	//init library
    if (!glfwInit()) {
        fprintf(stderr,"%s:Failed to initialize GLFW\n",__func__);
		exit(-1);
    }										

	//seet version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFW_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_MINOR);
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
		perror(__func__);
		if(wtSystem)
        	glfwTerminate();
		exit(-1);
	}
	memset(class,0,sizeof(WindowClass));
	
	//malloc name string
	class->windowName = malloc(strlen(name)+1);
	if(!class->windowName){
		perror(__func__);
		if(wtSystem)
        	glfwTerminate();
		exit(-1);
	}

	//copy data
	memcpy(&class->body,win,sizeof(Window));
	strcpy(class->windowName,name);

	//calc color
	char* color = (void*)&class->body.base_color;
	class->backColor.r = CHAR_TO_F(color[3])*(1.0f/255.0f);
	class->backColor.g = CHAR_TO_F(color[2])*(1.0f/255.0f);
	class->backColor.b = CHAR_TO_F(color[1])*(1.0f/255.0f);
	class->backColor.a = CHAR_TO_F(color[0])*(1.0f/255.0f);

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
		//set width & height
		if(WINDOW_IGNORE_SIZE & class->body.flag)
			class->body.width = class->body.height = 300;

		//create ftgl window
		class->window = glfwCreateWindow(class->body.width,class->body.height,class->windowName, NULL, NULL);
    	if (!class->window) {
        	fprintf(stderr,"%s:Failed to create window 1\n",__func__);
			if(wtSystem)
        		glfwTerminate();
        	exit(-1);
    	}
		GLFWwindow* befor = glfwGetCurrentContext();
		glfwMakeContextCurrent(class->window);
		
		//init shader
		if(!_wtInitShader()){
		fprintf(stderr,"%s:Faild init shader\n",__func__);
			exit(-1);
		}

		//generateShader
		if(!_wtGenerateShader(&class->shaderProgram,_wtVertexShaderSource,_wtFragmentShaderSource)){
			fprintf(stderr,"%s:Faild generate shader\n",__func__);
			exit(-1);
		}

		if(!_wtGenerateShader(&class->textShaderProgram,_wtTextVertexShaderSource,_wtTextFragmentShaderSource)){
			fprintf(stderr,"%s:Faild generate shader\n",__func__);
			exit(-1);
		}

		//attach
		glUseProgram(class->shaderProgram);

		//init ft
		if (FT_Init_FreeType(&class->fontLibrary)){
			fprintf(stderr,"%s:Failed to initialize Freetype\n",__func__);
			exit(-1);
		}

		//init pos
		if(!(WINDOW_IGNORE_POSITION & class->body.flag))
			glfwSetWindowPos(class->window,class->body.x,class->body.y);

		//setColor
		glClearColor(class->backColor.r,class->backColor.g,class->backColor.b,class->backColor.a);
	
		//sync window size
		glfwGetWindowSize(class->window,&class->body.width,&class->body.height);

		//calc scale
		int width,height;
		glfwGetFramebufferSize(class->window,&width,&height);
		class->hscale = width/((float)class->body.width);
		class->vscale = height/((float)class->body.height);

		//set global pos
		class->right = class->body.width;
		class->top  = class->body.height;
		
		//regist callback
		//glfwSetWindowSizeCallback(class->window,_glfw_callback_windowsizefun);
		glfwSetWindowPosCallback(class->window,_glfw_callback_windowposfun);
		glfwSetFramebufferSizeCallback(class->window,_glfw_callback_framebuffersizefun);
		glfwSetCursorPosCallback(class->window,_glfw_callback_cursorposition);

		//set interval
		glfwSwapInterval(1);

		//undo
		glfwMakeContextCurrent(befor);
	
		//push list
		LINEAR_LIST_PUSH(wtSystem->windowList,class);
		}

	//call
	class->body.callback(WINDOW_MESSAGE_CREATE,class,0,0,class->body.userData);

	//resize
	class->body.callback(WINDOW_MESSAGE_RESHAPE,class,
			(((uint64_t)class->body.width<<32))|(class->body.height&0xFFFFFFFF),
			0,class->body.userData);

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

void wtDrawSquare(WINDOW_HANDLE handle,int x,int y,int width,int height){
	WindowClass *class = handle;
	WindowClass *parent = (WindowClass*)class->body.parent;

	if(parent){
		if(class->left <= parent->left)
			x += class->left - parent->left;
		if(class->bottom <= parent->bottom)
			y += class->bottom - parent->bottom;
	}

	x -= (class->body.width >>1);
	y -= (class->body.height>>1);

	float x2 = ((x+width ) / (float)(class->body.width  >>1 )) * class->hscale;
	float y2 = ((y+height) / (float)(class->body.height >>1)) * class->vscale;
	float x1 = (x / (float)(class->body.width  >> 1)) * class->hscale;
	float y1 = (y / (float)(class->body.height >> 1)) * class->vscale;

	//create data
	GLfloat points[4][2] = {{x1,y1},{x2,y1},{x2,y2},{x1,y2}};
	unsigned char index[4] = {0,1,3,2};
	
	//generate vao & vbo
	unsigned int vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    //bind vao & vbo
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points),points, GL_STATIC_DRAW);

    //attach
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);
    glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN,0,4);

    //dettach
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);	
	
	//deleate
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
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

	x -= (class->body.width >>1);
	y -= (class->body.height>>1);

	GLfloat x1 = (x / (GLfloat)(class->body.width  >> 1)) * class->hscale;
	GLfloat y1 = (y / (GLfloat)(class->body.height >> 1)) * class->vscale;
	
	GLfloat* points = malloc(((pieces + 2)<<1) * sizeof(GLfloat));
	
	//center point
	points[0] = x1;
	points[1] = y1;

	radius <<= 1;
	GLfloat rad = 0;
	GLfloat hscaleRadius = (radius / (GLfloat)class->body.width ) * class->hscale;
	GLfloat vscaleRadius = (radius / (GLfloat)class->body.height) * class->vscale;

	int i;
	GLfloat deltaRad = (2*M_PI)/pieces;
	for(i = 1;i <= pieces;i++){
		points[(i<<1)  ] = x1 + cos(rad)*hscaleRadius;
		points[(i<<1)+1] = y1 + sin(rad)*vscaleRadius;

		rad += deltaRad;
	}

	points[((pieces+1)<<1)    ] = points[2];
	points[((pieces+1)<<1) + 1] = points[3];
	
	//generate vao & vbo
	unsigned int vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    //bind vao & vbo
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat) * ((pieces + 2)<<1) ,points, GL_STATIC_DRAW);

    //attach
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,0,0);
    glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN,0,pieces + 2);

    //dettach
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);	
	
	//deleate
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

	free(points);
}

void wtDrawText(WINDOW_HANDLE handle,int x,int y,char* str){
	WindowClass *class = handle;
	WindowClass *parent = (WindowClass*)class->body.parent;

	glEnable(GL_BLEND);
	wtSetColor4f(handle,1.0f,0.0f,0.0f,1.0f);
	render_text("ASOBO", 0.0f, 0.0f, 1.0f);
	glDisable(GL_BLEND);
	return;

	if(parent){
		if(class->left <= parent->left)
			x += class->left - parent->left;
		if(class->bottom <= parent->bottom)
			y += class->bottom - parent->bottom;
	}

	x -= (class->body.width>>1);
	y -= (class->body.height>>1);

	float fx = (x / (float)(class->body.width>>1 )) * class->hscale;
	float fy = (y / (float)(class->body.height>>1)) * class->vscale;

	glRasterPos2f(fx,fy);
}

void wtWindowSetTopWindow(WINDOW_HANDLE handle){
	WindowClass *class = handle;


	if(class->body.parent){
		WindowClass** itr;
		LINEAR_LIST_FOREACH(((WindowClass*)class->body.parent)->children,itr){
			if((*itr) == class){
				//get next 
				WindowClass** next = LINEAR_LIST_NEXT(itr);
				
				if(next){//if next availeble
					//get last and prev
					WindowClass** last = LINEAR_LIST_LAST(itr);
					WindowClass** prev = LINEAR_LIST_PREV(itr);

					//get pointer to itr
					WindowClass** next_prev = LINEAR_LIST_PREV(next);
					WindowClass** prev_next = LINEAR_LIST_NEXT(prev);
					//over writ pointer 
					next_prev = prev;
					prev_next = next;

					//push last
					WindowClass** last_next = LINEAR_LIST_NEXT(last);
					WindowClass** itr_prev  = LINEAR_LIST_PREV(prev);
					last_next = itr;
					itr_prev  = last;
				}
				return;
			}
		}
	}
	else{
	}
}

void wtMoveWindow(WINDOW_HANDLE handle,int x,int y){
	WindowClass *class = handle;

	class->body.x  = x;
	class->body.y  = y;

	if(class->body.parent == NULL){
		glfwSetWindowPos(class->window,x,y);
	}else{
		class->body.callback(WINDOW_MESSAGE_MOVE,class,
			(((uint64_t)x<<32))|(y&0xFFFFFFFF),0,class->body.userData);
		calcGrobalPos(class);
	}
}

void wtResizeWindow(WINDOW_HANDLE handle,int width,int height){
	WindowClass *class = handle;

	class->body.width  = width;
	class->body.height = height;

	if(class->body.parent == NULL){
		glfwSetWindowSize(class->window,width,height);

		//setData
		class->right = width;
		class->top   = height;
	}else{
		calcGrobalPos(class);
		class->body.callback(WINDOW_MESSAGE_RESHAPE,class,
			(((uint64_t)width<<32))|(height&0xFFFFFFFF),0,class->body.userData);
	}
}

void wtSetColor4f(WINDOW_HANDLE handle,float r,float g,float b,float a){
	WindowClass* class = handle;
	GLint loc = glGetUniformLocation(class->shaderProgram,"userSetColor");
	glUniform4f(loc,r,g,b,a);
}

static void wtWindowLoop(WindowClass** itr){
	if(itr == NULL)
		return;
	if(glfwWindowShouldClose((*itr)->window)){//if pushed closed button
		//call destroy callback
		if(!(*itr)->body.callback(WINDOW_MESSAGE_DESTROY,*itr,0,0,(*itr)->body.userData)){//ret 0
			//destroy window
			glfwDestroyWindow((*itr)->window);
			wtDeleateWindowAll(*itr);
			
			//call next
			wtWindowLoop(LINEAR_LIST_NEXT(itr));

			//delete element
			LINEAR_LIST_DELETE(itr);
		}
	}else{	
		//set Current
		glfwMakeContextCurrent((*itr)->window);

		//clear
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		//callback
		glViewport(0,0,(*itr)->body.width*(*itr)->hscale,(*itr)->body.height*(*itr)->vscale);
		(*itr)->body.callback(WINDOW_MESSAGE_DISPLAY,*itr,0,0,(*itr)->body.userData);
		
		//children callback
		callChildrenDisplay(*itr);

		//swap
		glfwSwapBuffers((*itr)->window);

		//call next
		wtWindowLoop(LINEAR_LIST_NEXT(itr));
	}
}

void wtMainLoop(){
	while(1){
		WindowClass** itr = LINEAR_LIST_NEXT(wtSystem->windowList);
		if(!itr){
			glfwTerminate();
			return;
		}

		wtWindowLoop(itr);
			
		//poll
		glfwWaitEventsTimeout(0.02);
	}
}

void wtSetCursor(WINDOW_HANDLE handle,uint8_t mode){
}

void wtLoadASCIIFont(WINDOW_HANDLE handle,const char* const path){
	WindowClass* class = handle;

	_wtLoadASCII(class->fontLibrary,path);
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
		wtSetColor4f((WINDOW_HANDLE)class,(*itr)->backColor.r,(*itr)->backColor.g,
			(*itr)->backColor.b,(*itr)->backColor.a);

		//draw base
		if(class->backColor.a > (1/(float)2550))
			wtDrawSquare(class,0,0,class->body.width,class->body.height);

		//callback
		(*itr)->body.callback(WINDOW_MESSAGE_DISPLAY,*itr,0,0,(*itr)->body.userData);
		
		callChildrenDisplay(*itr);
	}
}

//Child window reshape
static void calcChildrenGlobalPos(WindowClass* class){
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


static void _glfw_callback_windowposfun(GLFWwindow *window, int xpos, int ypos){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(window == (*itr)->window){	
			(*itr)->body.x = xpos;
			(*itr)->body.y = ypos;

			(*itr)->body.callback(WINDOW_MESSAGE_MOVE,*itr,
				(((uint64_t)xpos<<32))|(ypos&0xFFFFFFFF),0,(*itr)->body.userData);
			break;
		}
	}
}

static void _glfw_callback_windowsizefun(GLFWwindow *window, int width, int height){
}

static void _glfw_callback_windowrefreshfun(GLFWwindow *window){
}

static void _glfw_callback_windowfocusfun(GLFWwindow *window, int focused){
}

static void _glfw_callback_windowiconifyfun(GLFWwindow *window, int iconified){
}

static void _glfw_callback_windowmaximizefun(GLFWwindow *window, int maximized){
}

static void _glfw_callback_framebuffersizefun(GLFWwindow *window, int width, int height){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(window == (*itr)->window){	

			glfwGetWindowSize(window,&(*itr)->body.width,&(*itr)->body.height);
			(*itr)->hscale = width  / (*itr)->body.width;
			(*itr)->vscale = height / (*itr)->body.height;

			(*itr)->right = (*itr)->body.width;
			(*itr)->top   = (*itr)->body.height;

			(*itr)->body.callback(WINDOW_MESSAGE_RESHAPE,*itr,
					(((uint64_t)(*itr)->body.width<<32))|((*itr)->body.height&0xFFFFFFFF),
					0,(*itr)->body.userData);
			
			calcGrobalPos((*itr));

			break;
		}
	}
}

static void _glfw_callback_windowcontentscalefun(GLFWwindow *window, float xscale, float yscale){
}

static void _glfw_callback_cursorposition(GLFWwindow* window, double xpos, double ypos){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(wtSystem->windowList,itr){
		if(window == (*itr)->window){
			int x = xpos;
			int y = ypos;

			y = (*itr)->body.height - y;
			
			WindowClass* owner = getPointOwner((*itr),x,y);
				
			x = (x - owner->left);		
			y = (y - owner->bottom);			

			owner->body.callback(WINDOW_MESSAGE_PMOTION,owner,
					(((uint64_t)x<<32))|(y&0xFFFFFFFF),0,owner->body.userData);
		}
	}
}

/*
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
}*/

static void calcGrobalPos(WindowClass* class){
	
	if(class->body.parent == NULL){
		calcChildrenGlobalPos(class);
	}else{
		calcChildrenGlobalPos(class->body.parent);
	}
	
}


static WindowClass* getPointOwner(WindowClass* class,int x,int y){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(class->children,itr){

		if((*itr)->left <= x && (*itr)->bottom <= y 
			&& (*itr)->right >= x && (*itr)->top >= y){
			return getPointOwner((*itr),x,y);
		}	
	}
	return class;
}

static void wtDeleateWindowAll(WindowClass* class){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(class->children,itr){
		wtDeleateWindowAll(*itr);
	}

	LINEAR_LIST_RELEASE(class->children);
	free(class);
}

