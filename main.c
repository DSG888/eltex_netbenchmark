/*
	Написать реализацию серверов с использованием [обычных потоков с 
блокирующими send/recv, select, poll, epoll] с двумя протоколами [TCP/UDP]. 
Обеспечить возможность нагрузочного тестирования сервера.


*/

#include "main.h"

volatile uint64_t tcp_counter = 0, udp_counter = 0, tcp_data = 0, udp_data = 0, errors = 0;
uint8_t numcpu;

int main(int argc, char * argv[]) {
	// Тут создается очень случайное зерно
	srand(time(0) + getpid());
	int mode = 0;
	const uint8_t numcpu = get_nprocs_conf();
	unsigned int port = DEFAULT_PORT;
	struct in_addr out_ip = {0};
	char* hostname = DEFAULT_IP;
	if (2 > argc || argc > 4)
		argerror:
		DieWithError("args error. example:\n ./sock s <ip> <port>\n ./sock c <ip> <port>\n");
	// Просто парсинг параметров
	if (strlen(argv[1]) != 1 || !(mode = (!strchr(server_key, argv[1][0])?0:SERVER_MODE) | (!strchr(client_key, argv[1][0])?0:CLIET_MODE)))
		goto argerror;
	if (argc >= 3)
		hostname = argv[2];
	if (argc == 4 && ((port = atoi(argv[3])) > 65535))
		goto argerror;
	fprintf(stdout, "Режим %s\n", mode==SERVER_MODE?"сервера":"клиента");
	// Резолв хоста через DNS
	if (1) {
		struct hostent* he;
		if ((he = gethostbyname(hostname)) == NULL)
			DieWithError("DNS сломан\n");
		struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;
		for(int i = 0; addr_list[i] != NULL; ++i) {
			out_ip = *addr_list[i];
			break;
		}
	}
	signal(SIGPIPE, SIG_IGN);	// На случай неудачной передачи

	if (mode == SERVER_MODE) {
		// Действия на SIGALRM
		struct sigaction act;
		act.sa_handler = load_meter;
		act.sa_flags = SA_RESTART;
		sigaction(SIGALRM, &act, NULL);
		
		fprintf(stdout, "Прослушивание: %s:%d\n", inet_ntoa(out_ip), port);
		int sock_t = create_server_tcp_socket(out_ip.s_addr, port);
		int sock_u = create_server_udp_socket(out_ip.s_addr, port, 1);
		
		// Задание времени срабатывания таймера
		struct itimerval timerval = {0};
		timerval.it_value.tv_sec = 1;
		timerval.it_interval.tv_sec = 1;
		setitimer(ITIMER_REAL, &timerval, 0);	// Установка самого таймера
		
		// Создание и инициализация глобального массива дескрипторов
		#ifndef PTHREAD
		// Установка ограничений
		#ifdef SELECT
			const uint32_t dsize = SOCK_PER_TH * numcpu;
		#elif POLL
			const uint32_t dsize = POLL_SIZE;
		#endif
		
		#ifndef EPOLL
		int* allsocks = malloc(sizeof(int) * dsize);
		for (uint16_t i = 0; i < dsize; ++i)
			allsocks[i] = -1;
		#endif
		srv_sock_thr_t* srv_sock_thr_arr = malloc(sizeof(srv_sock_thr_t) * numcpu);
		if (!srv_sock_thr_arr)
			DieWithError("malloc thr_struct\n");
		// Стартуем numcpu потоков для обработки входящих подключений
		for (int i = 0; i < numcpu; ++i) {
			#ifdef SELECT
				srv_sock_thr_arr[i].num = SOCK_PER_TH;
				srv_sock_thr_arr[i].allsock = allsocks + i * SOCK_PER_TH;
			#elif POLL
				const uint16_t items_per_th = dsize / numcpu;
				const uint16_t left = i * items_per_th;
				const uint16_t right = (i == numcpu - 1)?(dsize - 1):(left + items_per_th - 1);
				const uint16_t count = right - left + 1;
				srv_sock_thr_arr[i].num = count;
				srv_sock_thr_arr[i].allsock = allsocks + left;
			#elif EPOLL
				srv_sock_thr_arr[i].EPoll = epoll_create1(0);
				srv_sock_thr_arr[i].MasterSocket = sock_t;
			#endif
			srv_sock_thr_arr[i].cou = 0;
			srv_sock_thr_arr[i].maxdx = -1;
			if ((pthread_create(&(srv_sock_thr_arr[i].threadID), NULL, server_worker_multi_tcp, &srv_sock_thr_arr[i])) < 0)
				DieWithError("\npthread_create() failed");
		}
		uint16_t tc = 0;	// Переменная для распределения дескрипторов между потоками. Не может быть >= числа ядер
		#endif
		
		#ifdef PTHREAD
			printf(" Обработка на потоках. 1 клиент = 1 поток\n");
			// Создание потока для приема UDP сообщений
			pthread_t thread_handler;
			if (pthread_create(&thread_handler, NULL, server_handler_udp, &sock_u) < 0)
				DieWithError("thread_create\n");
			connect_node_t* newconnect = NULL;
			while (1) {
				// Создание потоков для каждого клиента c TCP запросом
				newconnect = malloc(sizeof(connect_node_t));
				if (!newconnect)
					DieWithError("memerror");
		//		newconnect->num = client_count++;
				//fprintf(stdout, " Жду подключения\n");
				newconnect->sock = accept_tcp_connection(sock_t);
				if ((newconnect->client_threadID = pthread_create(&newconnect->client_threadID, NULL, server_worker_tcp, (void*)newconnect)) < 0)
					DieWithError("\npthread_create() failed");
		//		ll_add_tail(threadlist, newconnect);
			}
		#elif SELECT
			printf(" Обработка на selectах\n");
			assert(1024 >= SOCK_PER_TH);	// select не может обрабатывать больше 1024 дескрипторов за раз
			fd_set fdstruct;
			FD_ZERO(&fdstruct);
			while (1) {
				FD_SET(sock_t, &fdstruct);
				FD_SET(sock_u, &fdstruct);
				int maxd = maxnum(sock_t, sock_u);
				if (select(maxd + 1, &fdstruct, NULL, NULL, NULL) == -1)
					//DieWithError("select!\n");
					{}//FIXME! Иногда работает
				if (FD_ISSET(sock_t, &fdstruct)) {printf("Появился клиент\n");
					// Пришел TCP клиент
					int newsock = accept_tcp_connection(sock_t);
					// Попытка сунуть дескриптор в поток
					for (int i = 0; i < numcpu; ++i) {
						if (srv_sock_thr_arr[tc].num <= srv_sock_thr_arr[tc].cou + 1) {
							if (++tc >= numcpu)
								tc = 0;
							continue;
						}
						else {
							for (int j = 0; j < srv_sock_thr_arr[tc].num; ++j) {
								// Находим в потоке свободный дескриптор
								if (srv_sock_thr_arr[tc].allsock[j] == -1) {
									srv_sock_thr_arr[tc].allsock[j] = newsock;
									if (srv_sock_thr_arr[tc].maxdx < newsock)
										srv_sock_thr_arr[tc].maxdx = newsock;
									newsock = 0;
									srv_sock_thr_arr[tc].cou++;
									break;
								}
							}
							if (++tc >= numcpu)
								tc = 0;
							if (!newsock)
								break;
						}
					}
					if (newsock) {
						fprintf(stderr, "Дескриптор клиента потерян\n");
						++errors;
					}
				}
				if (FD_ISSET(sock_u, &fdstruct)) {
					// Прием UDP прямо здесь
					char data[sizeof(DATA) + 10] = {'\0'};
					if ((recv(sock_u, &data, sizeof(DATA), 0)) != sizeof(DATA) || strcmp(DATA, data)) {
						++errors;
						continue;
					}
					else {
						++udp_counter;
						udp_data += sizeof(DATA);
					}
				}
			}
			free(allsocks);
			free(srv_sock_thr_arr);
		#elif POLL
			printf(" Обработка на poll\n");
			struct pollfd Set[2];
			Set[0].fd = sock_t;
			Set[1].fd = sock_u;
			Set[0].events = POLLIN;
			Set[1].events = POLLIN;
			while (1) {
				poll(Set, 2 + 1, -1);	//FIXME Проверка
				if (Set[0].revents & POLLIN) {
					// Пришел TCP клиент
					int newsock = accept_tcp_connection(sock_t);
					// Попытка сунуть дескриптор в поток
					for (int i = 0; i < numcpu; ++i) {
						if (srv_sock_thr_arr[tc].num <= srv_sock_thr_arr[tc].cou + 1) {
							if (++tc >= numcpu)
								tc = 0;
							continue;
						}
						else {
							for (int j = 0; j < srv_sock_thr_arr[tc].num; ++j) {
								// Находим в потоке свободный дескриптор
								if (srv_sock_thr_arr[tc].allsock[j] == -1) {
									srv_sock_thr_arr[tc].allsock[j] = newsock;
									if (srv_sock_thr_arr[tc].maxdx < newsock)
										srv_sock_thr_arr[tc].maxdx = newsock;
									newsock = 0;
									srv_sock_thr_arr[tc].cou++;
									break;
								}
							}
							if (++tc >= numcpu)
								tc = 0;
							if (!newsock)
								break;
						}
					}
					if (newsock) {
						fprintf(stderr, "Дескриптор клиента потерян\n");
						++errors;
					}//FIXME копия кода выше
				}
				if (Set[1].revents & POLLIN) {
					// Прием UDP прямо здесь
					char data[sizeof(DATA) + 10] = {'\0'};
					if ((recv(sock_u, &data, sizeof(DATA), 0)) != sizeof(DATA) || strcmp(DATA, data)) {
						++errors;
						continue;
					}
					else {
						++udp_counter;
						udp_data += sizeof(DATA);
					}//FIXME Копия кода выше
				}
				
			}
		#elif EPOLL
			printf(" Обработка на epoll\n");
			struct epoll_event event1, event2;
			
			int epolld = epoll_create1(0);	// Задание дескриптора
			
			event1.data.fd = sock_t;
			event2.data.fd = sock_u;
			event1.events = EPOLLIN;	// отслеживание доступности на чтение
			event2.events = EPOLLIN;	// отслеживание доступности на чтение
			epoll_ctl(epolld, EPOLL_CTL_ADD, sock_t, &event1);	// Регистрация TCP сокета
			epoll_ctl(epolld, EPOLL_CTL_ADD, sock_u, &event2);	// Регистрация UDP сокета
			
			
			while (1) {
				//struct epoll_event events[MAX_EVENTS];
				//uint8_t N = epoll_wait(epolld, events, MAX_EVENTS, -1);
				//for (uint8_t i = 0; i < N; ++i) {
					
				//}
				struct epoll_event events[2];
				int epollsize = epoll_wait(epolld, events, 2, -1);
				for (int i = 0; i < epollsize; ++i) {
					if (events[i].data.fd == sock_t) {
						// Пришел TCP клиент
						int newsock = accept_tcp_connection(sock_t);
						// Попытка сунуть дескриптор в поток
						for (int i = 0; i < numcpu; ++i) {
							// Распределение между потоками
							if (0) {
						//	if (srv_sock_thr_arr[tc].num <= srv_sock_thr_arr[tc].cou + 1) {
						//		if (++tc >= numcpu)
						//			tc = 0;
						//		continue;
							}
							else {
						//		for (int j = 0; j < srv_sock_thr_arr[tc].num; ++j) {
						//			// Находим в потоке свободный дескриптор
						//	//		if (srv_sock_thr_arr[tc].allsock[j] == -1) {
						//	//			srv_sock_thr_arr[tc].allsock[j] = newsock;
						//	{			if (srv_sock_thr_arr[tc].maxdx < newsock)
						//					srv_sock_thr_arr[tc].maxdx = newsock;
						//				newsock = 0;
						//				srv_sock_thr_arr[tc].cou++;
						//				break;
						//			}
						//		}
								struct epoll_event Event;
								set_nonblock(newsock);
								Event.data.fd = newsock;
								Event.events = EPOLLIN;
								epoll_ctl(srv_sock_thr_arr[tc].EPoll, EPOLL_CTL_ADD, newsock, &Event);
								newsock = 0;
								srv_sock_thr_arr[tc].maxdx = 1;	//Костыль
								if (++tc >= numcpu)
									tc = 0;
								if (!newsock)
									break;
							}
						}
						if (newsock) {
							fprintf(stderr, "Дескриптор клиента потерян\n");
							++errors;
						}//FIXME копия кода выше
					}
					if (events[i].data.fd == sock_u) {
						// Прием UDP прямо здесь
						char data[sizeof(DATA) + 10] = {'\0'};
						if ((recv(sock_u, &data, sizeof(DATA), 0)) != sizeof(DATA) || strcmp(DATA, data)) {
							++errors;
							continue;
						}
						else {
							++udp_counter;
							udp_data += sizeof(DATA);
						}//FIXME Копия кода выше
					}
				}
			}
			
			
		#else
			printf("Ошибка. Нет заданного обработчика\n");
		#endif
	}
	else {
		fprintf(stdout, "Используется хост '%s' -> %s:%d\n", hostname, inet_ntoa(out_ip), port);
		struct main_handler_args_s srv_args = {out_ip.s_addr, port};	//FIXME ZONE
		pthread_t thread_handler;
		if (pthread_create(&thread_handler, NULL, client_handler_tcp, &srv_args) < 0)
			DieWithError("thread_create\n");
		struct main_handler_args_s srv_args2 = {out_ip.s_addr, port};
		if (pthread_create(&thread_handler, NULL, client_handler_udp, &srv_args2) < 0)
			DieWithError("thread_create\n");
		
	}
	// Ну основной поток ничего делать не будет
	while (1) {
		sleep(1);
	}

	return EXIT_SUCCESS;
}
