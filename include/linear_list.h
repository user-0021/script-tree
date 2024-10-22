/**
 * 線形リストを扱うライブラリです。 
 * ただし、いくつかの機能のために安全性を捨てているため、
 * 理解して使わないと、意図せぬ動作が引き起こされます。
 * 
 * また、このライブラリはマクロの呼び出しのみを想定しているため、
 * 直接使われたくない関数の名前には先頭にdo_not_useが付きます。 
 */
#pragma once

void* do_not_use_list_get_last(void* list,unsigned int size);
void* do_not_use_list_create(unsigned int size,void* next,void* pre);
void do_not_use_list_release(void* list,unsigned int size);
void do_not_use_list_push(void* list,const void* const data,unsigned int size);

#define LINEAR_LIST_NEXT(list) (((void*)list)+sizeof(typeof(*list)))
#define LINEAR_LIST_PREV(list) (((void*)list)+sizeof(typeof(*list))+sizeof(void*))

#define LINEAR_LIST_CREATE(type) ((type*)do_not_use_list_create(sizeof(type),(void*)0,(void*)0))
#define LINEAR_LIST_RELEASE(list) do_not_use_list_release(list,sizeof(typeof(*list)))
#define LINEAR_LIST_PUSH(list,data) do_not_use_list_push(list,&data,sizeof(typeof(*list)))

#define LINEAR_LIST_LAST(list) do_not_use_list_get_last(list,sizeof(typeof(*list)))
#define LINEAR_LIST_FOREACH(list,iter) for(iter = *(void**)(((void*)list) + sizeof(*list));iter != NULL;iter = *(void**)LINEAR_LIST_NEXT(iter))
#define LINEAR_LIST_FOREACH_R(list,iter) for(iter = LINEAR_LIST_LAST(list);iter != list;iter = *(void**)LINEAR_LIST_PREV(iter))
