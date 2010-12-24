CC = gcc
CFLAGS = -Wall
LDFLAGS = -ltag_c

all: id3reset

id3reset: id3reset.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ id3reset.c

clean:
	rm id3reset

#run: id3reset
#	./id3reset mp3
