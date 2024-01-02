CC=gcc
CFLAGS=-O2 -Wall -std=c11
LDFLAGS=-lcrypto -lssl
INC=-I.

all: lp25-backup

%.o: %.c %.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ -lcrypto -lssl

file-properties.o: file-properties.c file-properties.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@ -lcrypto -lssl

lp25-backup: main.c files-list.o sync.o configuration.o file-properties.o processes.o messages.o utility.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(INC) -o $@ $^ -lcrypto -lssl

clean:
	rm -f *.o lp25-backup