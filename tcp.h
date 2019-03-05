#ifndef TCP_H
#define TCP_H

#include <stdio.h>		//fprintf
#include <pthread.h>	//pthread_detach pthread_self
#include <sys/socket.h>	//recv send
#include <unistd.h>		//sleep
#include <string.h>		//memset
#include <fcntl.h>		//FIOBIO


#include "other.h"
#include "main.h"

int create_server_tcp_socket(unsigned int ip, int port);
int accept_tcp_connection(int servSock);
int client_tcp_connect(unsigned int ip, int port);
void* server_worker_tcp(void *threadArgs);
void* server_handler_tcp(void *threadArgs);
void* client_handler_tcp(void *threadArgs);
int set_nonblock(int fd);
void* server_worker_multi_tcp(void *threadArgs);

//#ifndef PTHREAD
// Структура обработчика соединений
struct srv_sock_thr_s {
	uint16_t num;	// Общее число дескрипторов, которые отданы потоку
	uint16_t cou;	// Число занятых дескрипторов
	int maxdx;		// Максимальный дескриптор
	pthread_t threadID;
	#ifdef EPOLL
	int MasterSocket, EPoll;
	#else
	int* allsock;	// Указатель на первый дескриптор для этого потока
	#endif
} typedef srv_sock_thr_t;
//#endif

#endif
