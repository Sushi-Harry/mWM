CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LIBS = -lX11

SRCS = src/main.c src/core.c src/layout.c src/ipc.c
OBJS = $(SRCS:.c=.o)
TARGET = mwm

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

apps:
	$(CC) -Wall -Wextra -std=c99 tools/mwm-gamma.c -o mwm-gamma -lX11 -lXrandr -lm
	$(CC) -Wall -Wextra -std=c99 tools/mwm-tracker.c -o mwm-tracker -lX11 -lXss -lsqlite3
	g++ -std=c++17 -O2 -Iinclude tools/mwm-control-center.cpp -o mwm-control-center -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lsqlite3

.PHONY: all clean