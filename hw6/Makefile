all: main.c scheduler.c
	gcc -Wall -o main main.c scheduler.c threads.c
gdb: main.c scheduler.c
	gcc -Wall -o main main.c scheduler.c threads.c -g
clean:
	rm main
