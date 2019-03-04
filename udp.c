#include "udp.h"

extern uint64_t udp_data, udp_counter, errors;

int create_server_udp_socket(unsigned int ip, int port, int binding) {
	int sock = -1;
	struct sockaddr_in echoServAddr;
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	// Таймаут в 2 секунды на передачу
//	struct timeval timeV = {2, 0};
//	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeV, sizeof(timeV)) == -1) {
//		fprintf(stderr, "setsockopt (SO_RCVTIMEO)\n");
//		exit(EXIT_FAILURE);
//	}
	if (binding) {
		memset(&echoServAddr, 0, sizeof(echoServAddr));
		echoServAddr.sin_family = AF_INET;
		echoServAddr.sin_addr.s_addr = binding?htonl(INADDR_ANY):ip;
		echoServAddr.sin_port = htons(port);
		if (bind(sock, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) < 0)
			DieWithError("bind() failed");
	}
	#ifndef PTHREAD
	set_nonblock(sock);
	#endif
	return sock;
}

void* client_handler_udp(void *threadArgs) {
	pthread_detach(pthread_self());		// Guarantees that thread resources are deallocated upon return
	
	struct sockaddr_in echoServAddr;
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = (((struct main_handler_args_s*)threadArgs)->ip);
	echoServAddr.sin_port = htons(((struct main_handler_args_s*)threadArgs)->port);
	
	fprintf(stdout, "Основной поток клиента запущен\n");
	int sock = create_server_udp_socket(((struct main_handler_args_s*)threadArgs)->ip, ((struct main_handler_args_s*)threadArgs)->port, 0);
	while (1) {
		// Отправка запроса
		//fprintf(stdout, "  Отправка запроса");
		if ((sendto(sock, &DATA, sizeof(DATA), 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != sizeof(DATA))) {
			//fprintf(stdout, " не удалась\n");
			sleep(3);
			continue;
		}
		continue;
	}
	return NULL;
}

void* server_handler_udp(void *threadArgs) {
	pthread_detach(pthread_self());
	while (1) {
		char data[sizeof(DATA) + 10] = {'\0'};
		if ((recv(*((int*)threadArgs), &data, sizeof(DATA), 0)) != sizeof(DATA) || strcmp(DATA, data)) {
			++errors;
			continue;
		}
		++udp_counter;
		udp_data += sizeof(DATA);
	}
	return NULL;
}
