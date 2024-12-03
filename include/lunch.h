#pragma once
#include <stdint.h>

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
	NODE_DOUBLE		= 12,
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
}nodePipe;

typedef struct{
	int pid;
	int fd[3];
	char* name;
	uint16_t pipeCount;
	nodePipe* pipes;
}nodeData;


