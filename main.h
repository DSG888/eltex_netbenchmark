#ifndef MAIN_H
#define MAIN_H


#include <stdio.h>		//fprintf
#include <stdlib.h>		//exit srand EXIT_SUCCESS EXIT_FAILURE
#include <string.h>		//strlen strchr
#include <pthread.h>	//pthread_create
#include <netdb.h>		//gethostbyname hostent
#include <arpa/inet.h>	//inet_ntoa
#include <signal.h>		//signal
#include <unistd.h>		//sleep
#include <sys/sysinfo.h>	//get_nprocs_conf
#include <assert.h>		//assert

#include "linkedlist.h"
#include "other.h"
#include "tcp.h"
#include "udp.h"

#define server_key	"Ss"
#define client_key	"Cc"
#define SERVER_MODE	3
#define CLIET_MODE	4
#define DEFAULT_PORT 62000
#define DEFAULT_IP	"127.0.0.1"
#define MAXPENDING	10
//Очень важная строка 595 байта
#define DATA		"0123456789@АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя№AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789@АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя№AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789@АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя№AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz"

#ifdef SELECT
	#define SOCK_PER_TH	1024	// Число сокетов на один поток
#else
	#ifdef POLL
		#include <poll.h>
	#endif
	#define POLL_SIZE	2048	// Общее число сокетов
#endif



struct main_handler_args_s {	// Это можно было бы заменить более развесистой структурой struct sockaddr_in
	unsigned int ip;
	int port;
};

struct connect_node_s {
	unsigned int num;
	int sock;
	pthread_t client_threadID;
} typedef connect_node_t;

#endif

