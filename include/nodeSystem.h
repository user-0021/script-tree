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
	NODE_OUT	= 1
} NODE_PIPE_TYPE;

static const char* NODE_PIPE_TYPE_STR[3] = {
	"IN",
	"OUT"
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

static const uint16_t NODE_DATA_UNIT_SIZE[13] = {
	0,
	sizeof(char),
	1,
	sizeof(int8_t),
	sizeof(int16_t),
	3,
	sizeof(int32_t),
	sizeof(uint8_t),
	sizeof(uint16_t),
	3,
	sizeof(uint32_t),
	sizeof(float),
	sizeof(double)
};

typedef struct{
	char* pipeName;
	uint16_t length;
	NODE_PIPE_TYPE type;
	NODE_DATA_UNIT unit;
	char* connectPipe;
	int sID;
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
#define NODE_SYSTEM_NONE_SUCH_THAT -4
#define NODE_SYSTEM_INVALID_ARGS -5

