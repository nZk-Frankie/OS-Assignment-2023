CC = gcc
CFLAGS = -Wall -g
TARGET = cq

all : $(TARGET)

cq : main.o
	$(CC) $(CFLAGS) main.c -o $(TARGET)

main.o : main.c main.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm $(TARGET) main.o
