#pragma once

typedef struct{
	char* name;
	void (*func)(int* argc,char* argv[]);
	char* man;
} Command;
