VAR = PTHREAD
VAR = SELECT
VAR = POLL
VAR = EPOLL

CC = gcc
CFLAGS = -std=c99 -O0 -g2 -Wall -D$(VAR) -D_POSIX_C_SOURCE=200809
LDFLAGS = -lpthread
SOURCES = main.c other.c tcp.c udp.c linkedlist.c
OBJECT_FILES = $(addprefix obj/, $(SOURCES:.c=.o))
EXECUTABLE = sock

all: obj $(SOURCES) $(EXECUTABLE)

obj:
	mkdir obj

$(EXECUTABLE): $(OBJECT_FILES)
	$(CC) $(OBJECT_FILES) $(LDFLAGS) $(CFLAGS) -o $@

obj/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -rf obj $(EXECUTABLE)

#test: all
#	@echo "Запуск интерактивного тестирования"
#	xterm -title "TCP Сервер: $(EXECUTABLE) st 127.0.0.1 5000" -geometry 80x24+0+0     -e "${PWD}/$(EXECUTABLE) st 127.0.0.1 5000" &
#	xterm -title "TCP Клиент: $(EXECUTABLE) ct 127.0.0.1 5000" -geometry 80x24+500+0   -e "${PWD}/$(EXECUTABLE) ct 127.0.0.1 5000" &
#	xterm -title "UDP Сервер: $(EXECUTABLE) su 127.0.0.1 6000" -geometry 80x24+0+380   -e "${PWD}/$(EXECUTABLE) su 127.0.0.1 6000" &
#	xterm -title "UDP Клтент: $(EXECUTABLE) cu 127.0.0.1 6000" -geometry 80x24+500+380 -e "${PWD}/$(EXECUTABLE) cu 127.0.0.1 6000" &
#	sleep 0.3
#	o=2;t=2; while clear; echo "Интерактивное тестирование:"; echo " [1] Запуск TCP клиента"; echo " [2] Запуск UDP клиента"; echo " [*] Завершить тест"; read -p "> " q; do if [ "$$q" = "1" ] ; then o=$$(($$o+1)); xterm -title "TCP Клиент №$$o: $(EXECUTABLE) ct 127.0.0.1:5000" -geometry 80x24+$$((($$o-1)*500))+0 -e "${PWD}/$(EXECUTABLE) ct 127.0.0.1 5000" & else if [ "$$q" = "2" ] ; then t=$$(($$t+1)); xterm -title "UDP Клиент №$$t: $(EXECUTABLE) cu 127.0.0.1:6000" -geometry 80x24+$$((($$t-1)*500))+380 -e "${PWD}/$(EXECUTABLE) cu 127.0.0.1 6000" & else break; fi; fi; done
#	killall $(EXECUTABLE)
#	clear
