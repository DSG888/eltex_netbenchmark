#include "linkedlist.h"

ll_t* ll_create() {
	ll_t* list = (ll_t*)malloc(sizeof(ll_t));
	if (!list)
		return NULL;
	list->count = 0;
	list->head = NULL;
	list->tail = NULL;
	pthread_mutex_init(&(list->mutex), NULL);
	return list;
}

void ll_free(ll_t* list) {
	pthread_mutex_lock(&(list->mutex));
	if (!list) 
		return;
	ll_node_t* li = list->head;
	while (li != NULL) {
		struct ll_node_s* tmp = li->next;
		free(li);
		li = tmp;
	}
	pthread_mutex_unlock(&(list->mutex));
	pthread_mutex_destroy(&(list->mutex));
	free(list);
	return;
}

int ll_add_tail(ll_t* list, void* ptr) {	//fixed
	pthread_mutex_lock(&(list->mutex));
	if (!list)
		return 1;
	ll_node_t* li = (ll_node_t *)malloc(sizeof(ll_node_t));
	if (!li)
		return 1;
	li->value = ptr;
	li->next = NULL;
	li->prev = list->tail;
	if (list->tail) {
		list->tail->next = li;
		list->tail = li;
	}
	else
		list->head = list->tail = li;
	list->count++;
	pthread_mutex_unlock(&(list->mutex));
	return 0;
}

int ll_add_head(ll_t* list, void* ptr) {
	pthread_mutex_lock(&(list->mutex));
	if (!list)
		return 1;
	ll_node_t* li = (ll_node_t *)malloc(sizeof(ll_node_t));
	if (!li)
		return 1;
	li->value = ptr;
	li->next = list->head;
	li->prev = NULL;
	if (list->head) {
		list->head->prev = li;
		list->head = li;
	}
	else
		list->tail = list->head = li;
	list->count++;
	pthread_mutex_unlock(&(list->mutex));
	return 0;
}

void* ll_get_index(ll_t* list, int index) {
	pthread_mutex_lock(&(list->mutex));
	if (!list)
		return NULL;
	if (index < 0 || list->count < index)
		return NULL;
	struct ll_node_s* tmp = list->head;
	for (int i = 0; i < index && tmp; ++i, tmp = tmp->next);
	pthread_mutex_unlock(&(list->mutex));
	return tmp->value;
	return 0;
}

int ll_del_tail(ll_t* list) {
	if (!list)
		return 1;
	pthread_mutex_lock(&(list->mutex));
	int res = 1;
	if (list->tail) {
		struct ll_node_s* tmp = list->tail;
		if (list->tail->prev)
			list->tail->prev->next = NULL;
		else
			list->head = NULL;
		list->tail = list->tail->prev;
		free(tmp);
		list->count--;
		res = 0;
	}
	pthread_mutex_unlock(&(list->mutex));
	return res;
}

int ll_del_head(ll_t* list) {
	if (!list)
		return 1;
	pthread_mutex_lock(&(list->mutex));
	int res = 1;
	if (list->head) {
		ll_node_t* tmp = list->head;
		if (list->head->next)
			list->head->next->prev = NULL;
		else
			list->tail = NULL;
		list->head = list->head->next;
		free(tmp);
		list->count--;
		res = 0;
	}
	pthread_mutex_unlock(&(list->mutex));
	return res;
}

#ifdef DEBUG
void ll_print(ll_t* list) {
	printf("\nlist_len: %d\thead_addr=[%p]\ttail_addr=[%p]\n=====\n", list->count, list->head, list->tail);
	ll_node_t* li = list->head;
	while (li != NULL) {
		printf("\tnode_addr=[%p],\tprev_addr=[%p],\tnext_addr=[%p],\tprt_addr=[%p]\n", li, li->prev, li->next, li->value);
		li = li->next;
	}
	printf("=====\n");
}
#endif
