CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGETS = hexdump reverse prun pipe2 worker timeserver timeclient echoserver echoclient

all: $(TARGETS)

hexdump: code/hexdump.c
	$(CC) $(CFLAGS) code/hexdump.c -o hexdump

reverse: code/reverse.c
	$(CC) $(CFLAGS) code/reverse.c -o reverse

prun: code/prun.c
	$(CC) $(CFLAGS) code/prun.c -o prun

pipe2: code/pipe2.c
	$(CC) $(CFLAGS) code/pipe2.c -o pipe2

worker: code/worker.c
	$(CC) $(CFLAGS) code/worker.c -o worker

timeserver: code/timeserver.c
	$(CC) $(CFLAGS) code/timeserver.c -o timeserver

timeclient: code/timeclient.c
	$(CC) $(CFLAGS) code/timeclient.c -o timeclient

echoserver: code/echoserver.c
	$(CC) $(CFLAGS) code/echoserver.c -o echoserver

echoclient: code/echoclient.c
	$(CC) $(CFLAGS) code/echoclient.c -o echoclient

clean:
	rm -f $(TARGETS)

.PHONY: all clean