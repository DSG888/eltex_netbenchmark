#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <pthread.h>
#include <stdlib.h>

typedef struct ll_node_s {
	void* value;
	struct ll_node_s* prev;
	struct ll_node_s* next;
} ll_node_t;

typedef struct {
	int count;
	ll_node_t* head;
	ll_node_t* tail;
	pthread_mutex_t mutex;
} ll_t;

ll_t *ll_create();
void ll_free(ll_t *list);
int ll_add_tail(ll_t* list, void* ptr);
int ll_add_head(ll_t* list, void* ptr);
int ll_del_tail(ll_t* list);
int ll_del_head(ll_t* list);
#ifdef DEBUG
#include <stdio.h>
void ll_print(ll_t* list);
#endif
#endif
