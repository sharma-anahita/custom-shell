CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = main.exe
OBJS = main.o helpers.o input_parser.o builtin.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

main.o: main.c shelly.h
	$(CC) $(CFLAGS) -c main.c

helpers.o: helpers.c shelly.h
	$(CC) $(CFLAGS) -c helpers.c

input_parser.o: input_parser.c shelly.h
	$(CC) $(CFLAGS) -c input_parser.c

builtin.o: builtin.c shelly.h
	$(CC) $(CFLAGS) -c builtin.c

clean:
	rm -f *.o $(TARGET)
