CFLAGS += -Wall -O2 -DNDEBUG
LDFLAGS += -lbcm2835
PREFIX=/usr/local/sbin

.PHONY: all install clean

all: dht11

dht11: dht11.o
	$(CC) -o $@ $^ ${LDFLAGS}

install: all
	install -d ${DESTDIR}${PREFIX}
	install dht11 ${DESTDIR}${PREFIX}

clean:
	$(RM) dht11 *.o
