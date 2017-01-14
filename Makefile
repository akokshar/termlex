
CC=gcc

_libs=vte-2.90

CFLAGS=$(shell pkg-config --cflags $(_libs))
LDLIBS=$(shell pkg-config --libs $(_libs))

#%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

all: termlex

termlex: termlex.c

clean:
	$(RM) *.o termlex

install:
	install -Dm755 termlex /usr/local/bin/termlex
