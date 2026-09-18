CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LIBS = -lX11

SRCS = src/main.c src/core.c
OBJS = $(SRCS:.c=.o)
TARGET = mwm

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean