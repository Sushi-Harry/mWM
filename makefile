CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -MMD -MP
CXXFLAGS = -std=c++17 -O2 -Iinclude


PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
XSESSIONSDIR = /usr/share/xsessions

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

-include $(DEPS)

all: mwm apps

mwm: $(OBJS)
	$(CC) $(OBJS) -o mwm -lX11

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

apps:
	$(CC) $(CFLAGS) tools/mwm-gamma.c -o mwm-gamma -lX11 -lXrandr -lm
	$(CC) $(CFLAGS) tools/mwm-tracker.c -o mwm-tracker -lX11 -lXss -lsqlite3
	$(CXX) $(CXXFLAGS) tools/mwm-control-center.cpp -o mwm-control-center -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lsqlite3

install: all
	install -Dm755 mwm $(DESTDIR)$(BINDIR)/mwm
	install -Dm755 mwm-gamma $(DESTDIR)$(BINDIR)/mwm-gamma
	install -Dm755 mwm-tracker $(DESTDIR)$(BINDIR)/mwm-tracker
	install -Dm755 mwm-control-center $(DESTDIR)$(BINDIR)/mwm-control-center
	install -Dm755 mwm-session $(DESTDIR)$(BINDIR)/mwm-session
	install -Dm644 mwm.desktop $(DESTDIR)$(XSESSIONSDIR)/mwm.desktop

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/mwm \
	      $(DESTDIR)$(BINDIR)/mwm-gamma \
	      $(DESTDIR)$(BINDIR)/mwm-tracker \
	      $(DESTDIR)$(BINDIR)/mwm-control-center \
	      $(DESTDIR)$(BINDIR)/mwm-session \
	      $(DESTDIR)$(XSESSIONSDIR)/mwm.desktop

clean:
	rm -f src/*.o mwm mwm-gamma mwm-tracker mwm-control-center src/*.d