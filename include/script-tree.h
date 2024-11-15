#pragma once

typedef struct{
	char* argName;
	void (*func)(int* argc,char* argv[]);
} Command;
