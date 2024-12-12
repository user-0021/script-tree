#pragma once 
#include <stdio.h>
#include <stdint.h>


int nodeSystemInit(uint8_t isNoLog);
int nodeSystemAdd(char* path,char** args);
void nodeSystemList(int* argc,char** args);
int nodeSystemConnect(char* const inNode,char* const inPipe,char* const outNode,char* const outPipe);

static const uint32_t _node_init_head = 0x83DFC690;
static const uint32_t _node_init_eof  = 0x85CBADEF;
static const uint32_t _node_begin_head = 0x9067F3A2;
static const uint32_t _node_begin_eof  = 0x910AC8BB;

typedef enum{
	NODE_IN		= 0,
	NODE_OUT	= 1,
	NODE_IN_OUT	= 2
} NODE_PIPE_TYPE;

static const char* NODE_PIPE_TYPE_STR[3] = {
	"IN",
	"OUT",
	"IN/OUT"
};

typedef enum{
	NODE_STR		= 0,
	NODE_CHAR		= 1,
	NODE_BOOL		= 2,
	NODE_INT_8		= 3,
	NODE_INT_16		= 4,
	NODE_INT_24		= 5,
	NODE_INT_32		= 6,
	NODE_UINT_8		= 7,
	NODE_UINT_16	= 8,
	NODE_UINT_24	= 9,
	NODE_UINT_32	= 10,
	NODE_FLOAT		= 11,
	NODE_DOUBLE		= 12
} NODE_DATA_UNIT;

static const char* NODE_DATA_UNIT_STR[13] = {
	"STR",
	"CHAR",
	"BOOL",
	"INT_8",
	"INT_16",
	"INT_24",
	"INT_32",
	"UINT_8",
	"UINT_16",
	"UINT_24",
	"UINT_32",
	"FLOAT",
	"DOUBLE"
};

typedef struct{
	char* pipeName;
	uint16_t length;
	NODE_PIPE_TYPE type;
	NODE_DATA_UNIT unit;
	int fd[2];
}nodePipe;

typedef struct{
	int pid;
	int fd[3];
	char* name;
	char* filePath;
	uint16_t pipeCount;
	nodePipe* pipes;
}nodeData;

#define NODE_SYSTEM_SUCCESS 0
#define NODE_SYSTEM_ALREADY -1
#define NODE_SYSTEM_FAILED_RUN -2
#define NODE_SYSTEM_FAILED_INIT -3

