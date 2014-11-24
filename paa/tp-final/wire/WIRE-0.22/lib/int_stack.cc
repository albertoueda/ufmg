#include "int_stack.h"


void int_stack_init(int_stack_t *s){
	s->pos = 0;
	for(int i=0; i<MAX_STACK_SIZE; i++)
		s->data[i]=0;
}


void int_stack_push(int_stack_t *s, int val){
	s->pos = (s->pos<MAX_STACK_SIZE-1?s->pos+1:0);
	s->data[s->pos]=val;
}


int int_stack_pop(int_stack_t *s){
	int ret = s->data[s->pos];
	s->pos = (s->pos>1?s->pos-1:MAX_STACK_SIZE-1);
	return ret;
}

int int_stack_peek(int_stack_t *s){
	return s->data[s->pos];
}
