#include <linear_list.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void* do_not_use_list_create(unsigned int size,void* next,void* pre){
	void* element = malloc(size + sizeof(void*)*2);
	memset(element,0,size + sizeof(void*)*2);

	*(void**)(element + size) = next;
	*(void**)(element + size + sizeof(void*)) = pre;

	if(next != NULL){
		*(void**)(next + size + sizeof(void*)) = element;
	}

	if(pre != NULL){
		*(void**)(pre + size) = element;
	}

	return element;
}

void do_not_use_list_release(void* list,unsigned int size){
	
	if(*(void**)(list + size) != NULL){
		do_not_use_list_release(*(void**)(list + size),size);
	}

	free(list);
}

void do_not_use_list_push(void* list,const void* const data,unsigned int size){
	void* element = do_not_use_list_create(size,(void*)0,do_not_use_list_get_last(list,size));
	memcpy(element,data,size);
}

void* do_not_use_list_get_last(void* list,unsigned int size){
	void* ite = list;

	while(*(void**)(ite + size) != NULL){
		ite = *(void**)(ite + size);
	}

	return ite;
}
