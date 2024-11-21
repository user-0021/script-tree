#pragma once
#include <stdint.h>

typedef enum{
	NODE_IN		= 0,
	NODE_OUT	= 1,
	NODE_IN_OUT	= 2
} NODE_PIPE_TYPE;

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

typedef struct{
	char* pipeName;
	uint16_t length;
	NODE_PIPE_TYPE type;
	NODE_DATA_UNIT unit;
}nodePipe;

typedef struct{
	int fd[3];
	char* name;
	uint16_t pipeCount;
	nodePipe* pipes;
}nodeData;


