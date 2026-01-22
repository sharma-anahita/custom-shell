CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = main.exe
OBJS = main.o input_parser.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

main.o: main.c shelly.h
	$(CC) $(CFLAGS) -c main.c

input_parser.o: input_parser.c shelly.h
	$(CC) $(CFLAGS) -c input_parser.c

clean:
	del /f *.o $(TARGET)
