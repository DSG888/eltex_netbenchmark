#include "other.h"

extern uint64_t tcp_counter, udp_counter, tcp_data, udp_data, errors;

int getrand(int min, int max) {
	return (double)rand() / (RAND_MAX + 1.0) * (max - min) + min;
}

/*double wtime() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}*/

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y) {
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec-x->tv_usec)/1000000+1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (y->tv_usec-x->tv_usec)/1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;
	return x->tv_sec < y->tv_sec;
}

void human_readable (uint64_t size, char* str) {
	double s = size;
	int i;
	for (i = 0; (i < DIMENSIONS_COUNT - 1 && s > 999); ++i, s/=1000);
	sprintf(str, "%0.3lf%s", s, DIMENSIONS[i]);
}

void load_meter(int sig) {
	if (sig == SIGALRM) {
		static struct timeval old = {0, 0};	// Старое время на процессоре
		struct timeval new = {0, 0};		// Новое время на процессоре
		struct timeval dif = {0, 0};		// 
		
		struct rusage usage;
		getrusage(RUSAGE_SELF, &usage);
		new = usage.ru_utime;
		
		timeval_subtract(&dif, &new, &old);
		char buf1[80] = {'\0'};
		char buf2[80] = {'\0'};
		human_readable(tcp_data, buf1);
		human_readable(udp_data, buf2);
		printf(" CPU: %ld.%06lds| TCP: %ld %s | UDP: %ld %s | ERR: %ld\n", dif.tv_sec, dif.tv_usec, tcp_counter, buf1, udp_counter, buf2, errors);
		// Обнуление глобальных счетчиков
		tcp_counter = 0;
		udp_counter = 0;
		tcp_data = 0;
		udp_data = 0;
		//errors = 0;
		
		old = new;
	}
	return;
}

void DieWithError(char *errorMessage) {
	if (errorMessage)
		fprintf(stderr, "%s\n", errorMessage);
	getchar();
	exit(EXIT_FAILURE);
}

