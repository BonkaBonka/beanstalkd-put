#
BEANSTALK_CLIENT?=beanstalk-client

BINS=bsc
LIBS=$(BEANSTALK_CLIENT)/libbeanstalk.a

CFLAGS=-Wall -O3 -g -std=c99 -I$(BEANSTALK_CLIENT)
LDFLAGS=$(LIBS)
PREFIX?=/usr/local

%: %.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

all:	$(BINS)

install:	all
	strip -s $(BINS)
	mkdir -p "$(PREFIX)/bin/"
	cp $(BINS) "$(PREFIX)/bin/"

clean:
	rm -f *.o $(BINS)
