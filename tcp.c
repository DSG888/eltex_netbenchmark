#include "tcp.h"

extern uint64_t tcp_counter, tcp_data, errors;

int create_server_tcp_socket(unsigned int ip, int port) {
	struct sockaddr_in server;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock) {
		fprintf(stderr, "Could not create socket\n");
		exit(EXIT_FAILURE);
	}
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = ip;
	server.sin_port = htons(port);
	
	if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
		fprintf(stderr, "bind failed\n");
		exit(EXIT_FAILURE);
	}
	#ifndef PTHREAD
	//set_nonblock(sock);
	#endif
	if (listen(sock, MAXPENDING) < 0) {
		fprintf(stderr, "listen failed\n");
		exit(EXIT_FAILURE);
	}
	return sock;
}

//#ifdef PTHREAD
int accept_tcp_connection(int servSock) {
	int clntSock;
	struct sockaddr_in echoClntAddr;
	unsigned int clntLen = sizeof(echoClntAddr);
	if ((clntSock = accept(servSock, (struct sockaddr*) &echoClntAddr, &clntLen)) < 0)
		DieWithError("accept() failed");
	return clntSock;
}
//#endif

int client_tcp_connect(unsigned int ip, int port) {
	int sock;
	struct sockaddr_in client;
	sock = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
	if (sock == -1) {
		fprintf(stderr, "Could not create socket\n");
	//	sleep(1);
	//	return 0;
		exit(EXIT_FAILURE);
	}
	memset(&client, 0, sizeof(client));
	client.sin_addr.s_addr = ip;
	client.sin_family = AF_INET;
	client.sin_port = htons(port);
	if (connect(sock, (struct sockaddr*)&client , sizeof(client)) < 0) {
		return -1;
	}
	return sock;
}

// Поток сервера, обрабатывающий только одно соединение
void* server_worker_tcp(void *threadArgs) {
	pthread_detach(pthread_self()); 
	int sock = ((struct connect_node_s*)threadArgs)->sock;
	while (1) {
		char buf[sizeof(DATA) + 10] = {'\0'};
		if (((recv(sock, buf, sizeof(DATA), 0)) != sizeof(DATA)) || strcmp(DATA, buf)) {
			++errors;
			break;
		}
		else {
			++tcp_counter;
			tcp_data += sizeof(DATA);
		}
	}
	close(sock);
	//shutdown()
	
	return NULL;
}

#ifndef PTHREAD
int set_nonblock(int fd) {
	int flags;
	#ifdef O_NONBLOCK
		if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
			flags = 0;
		return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	#else
		flags = 1;
		return ioctl(fd, FIOBIO, &flags);
	#endif
}

// Поток сервера, обрабатывающий много соединений
void* server_worker_multi_tcp(void *threadArgs) {
	pthread_detach(pthread_self());
	struct srv_sock_thr_s* srv_sock_thr = (struct srv_sock_thr_s*)threadArgs;
	
	#ifdef SELECT
		fd_set readfds;	//cpFIX readfds
	#elif POLL
		struct pollfd* Set = malloc(sizeof(struct pollfd) * srv_sock_thr->num); //FIXME malloc
	#endif
	while (1) {
		// Если нечего обрабатывать, то спать
		if (srv_sock_thr->maxdx == -1) {
			sleep(1);
			continue;
		}
		for (int i = 0, c = 0; i < srv_sock_thr->num; ++i) {
			if (srv_sock_thr->allsock[i] != -1) {
				#ifdef SELECT
					FD_SET(srv_sock_thr->allsock[i], &readfds);
					++c;
				#elif POLL
					Set[c].fd = srv_sock_thr->allsock[i];
					Set[c++].events = POLLIN;
				#endif
			}
			if (c >= srv_sock_thr->cou)
				break;
		}
		
		
		#ifdef SELECT
		if (select(srv_sock_thr->maxdx + 1, &readfds, NULL, NULL, NULL) == -1){}else{}
		for (int i = 0; i < srv_sock_thr->num; ++i) {
			if (srv_sock_thr->allsock[i] == -1)
				continue;
			if (FD_ISSET(srv_sock_thr->allsock[i], &readfds)) {
				int sock = srv_sock_thr->allsock[i];
		#elif POLL
		poll(Set, srv_sock_thr->cou + 1, -1);	//FIXME Проверка
		for (int i = 0; i < srv_sock_thr->cou; ++i) {
			if (Set[i].revents & POLLIN) {
				int sock = Set[i].fd;
		#endif
				uint16_t recvsize = 0;
				char buf[sizeof(DATA) + 10] = {'\0'};
				if (((recvsize = recv(sock, buf, sizeof(DATA), 0)) != sizeof(DATA)) || strcmp(DATA, buf)) {
					++errors;
					close(sock);
					#ifdef SELECT
					//if (sock == srv_sock_thr->maxdx) {
						//TODO НАЙТИ ДРУГОЙ МАКСИМАЛЬНЫЙ
					//}
					srv_sock_thr->allsock[i] = -1;
					#endif
					//FIXME Обработка событий закрытия сокета у POLL
					continue;
				}
				else {
					++tcp_counter;
					tcp_data += sizeof(DATA);
				}
			}
		}
	}
	return NULL;
}
#endif

// Поток TCP клиента
void* client_handler_tcp(void *threadArgs) {
	pthread_detach(pthread_self());
	
	while (1) {
		//fprintf(stdout, " Подключение к %s:%d\t", inet_ntoa(ip), ((struct main_handler_args_s*)threadArgs)->port);
		int sock = client_tcp_connect(((struct main_handler_args_s*)threadArgs)->ip, ((struct main_handler_args_s*)threadArgs)->port);
		if (0 > sock) {
			//fprintf(stdout, "[FAIL]\n");
			//sleep(3);
			continue;
		}
		//fprintf(stdout, "[ОК]\n");
		// Получение строки
		while (1) {
			if ((send(sock, DATA, sizeof(DATA), 0)) !=  sizeof(DATA)) { 
				close(sock);
				break;
			}
		}
	}
	return NULL;
}
