#include <window-tree.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <linear_list.h>
#include <FTGL/ftgl.h>

#define CHAR_TO_F(c) ((float)(0xFF & (unsigned int)c))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

typedef struct _window_class{
	int windowID;
	float vscale;
	float hscale;
	int left;
	int right;
	int top;
	int bottom;

	Window body;
	struct _window_class** children;
}WindowClass;


typedef struct{
	WindowClass** windowList;
}WindowSystemClass;

WindowSystemClass* wtSystem = NULL;

void glDisplay(void);
void glReShape(int w,int h);
void glKeyboard(unsigned char key, int x, int y);
void glMouse(int button, int state, int x ,int y);
void glMotion(int x, int y);
void glPassiveMotion(int x, int y);
void applyFlag(WindowClass* class);
WindowClass* getPointOwner(WindowClass* class,int x,int y);

//init window system
WindowSystem wtInit(int* argcp,char** argv){//init env data
	WindowSystemClass* class = malloc(sizeof(WindowSystemClass));
	memset(class,0,sizeof(WindowSystemClass));

	glutInit(argcp,argv);
	glutInitDisplayMode(GLUT_RGBA);
	
	class->windowList = LINEAR_LIST_CREATE(WindowClass*);

	wtSystem = class;

	return (WindowSystem)class;
}

//create Window
WINDOW_HANDLE wtCreateWindow(Window* win,char* name){
	WindowClass* class = malloc(sizeof(WindowClass));
	memset(class,0,sizeof(WindowClass));

	//copy data
	memcpy(&class->body,win,sizeof(Window));

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
		//create glut window
		class->windowID = glutCreateWindow(name);
		
		//setColor
		char* color = (void*)&class->body.base_color;
		glClearColor(CHAR_TO_F(color[3])*(1.0f/255.0f),CHAR_TO_F(color[2])*(1.0f/255.0f)
					,CHAR_TO_F(color[1])*(1.0f/255.0f),CHAR_TO_F(color[0])*(1.0f/255.0f));

		//init pos size
		glutInitWindowPosition(class->body.x,class->body.y);
		glutInitWindowSize(class->body.width,class->body.height);

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

void wtDrawSquare(WINDOW_HANDLE handle,int x,int y,int width,int height){
	WindowClass *class = handle;

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

void wtDrawText(WINDOW_HANDLE handle,int x,int y,char* str,FTGLfont* font){
	WindowClass *class = handle;

	x *= class->hscale;
	y *= class->vscale;

	glRasterPos2i(x,y);
	ftglRenderFont(font, str, FTGL_RENDER_ALL);
}
//Child window display
void callChildrenDisplay(WindowClass* class){
	WindowClass** itr;
	LINEAR_LIST_FOREACH(class->children,itr){
		//check pos
		if((*itr)->left < class->left || (*itr)->bottom < class->bottom
				|| (*itr)->left > class->right || (*itr)->bottom > class->top)
			return;

		//init view port
		glViewport((*itr)->left,(*itr)->bottom,(*itr)->right - (*itr)->left,(*itr)->top - (*itr)->bottom);
		
		//set Color
		char* color = (void*)&(*itr)->body.base_color;
		glColor4f(CHAR_TO_F(color[3])*(1.0f/255.0f),CHAR_TO_F(color[2])*(1.0f/255.0f)
					,CHAR_TO_F(color[1])*(1.0f/255.0f),CHAR_TO_F(color[0])*(1.0f/255.0f));
		


		//fill base
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

	glFlush();
}

//Child window reshape
void callChildrenReshape(WindowClass* class){
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
		(*itr)->right = MIN((*itr)->right,class->right);
		(*itr)->top   = MIN((*itr)->top,class->top);

		callChildrenReshape(*itr);
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
			callChildrenReshape(*itr);
		
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
			
			x = (x - (*itr)->left) * (*itr)->hscale;		
			y = (y - (*itr)->bottom) * (*itr)->vscale;		

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
				
			x = (x - (*itr)->left) * (*itr)->hscale;		
			y = (y - (*itr)->bottom) * (*itr)->vscale;			

			owner->body.callback(WINDOW_MESSAGE_MOUSE,owner,
					(((uint64_t)x<<32))|(y&0xFFFFFFFF),state,owner->body.userData);
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
				
			x = (x - (*itr)->left) * (*itr)->hscale;		
			y = (y - (*itr)->bottom) * (*itr)->vscale;			

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
				
			x = (x - (*itr)->left) * (*itr)->hscale;		
			y = (y - (*itr)->bottom) * (*itr)->vscale;			

			owner->body.callback(WINDOW_MESSAGE_PMOTION,owner,
					(((uint64_t)x<<32))|(y&0xFFFFFFFF),0,owner->body.userData);
		}
	}
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
