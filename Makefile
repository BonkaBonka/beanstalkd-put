#

CFLAGS?=-Wall -pedantic -OS -g -std=c99
PREFIX?=/usr/local

BEANSTALK_CLIENT?=beanstalk-client

$(BEANSTALK_CLIENT)/beanstalk.h:
	git submodule init
	git submodule update

$(BEANSTALK_CLIENT)/libbeanstalk.a:
	cd $(BEANSTALK_CLIENT)
	make libbeanstalk.a

bsc: bsc.c $(BEANSTALK_CLIENT)/beanstalk.h $(BEANSTALK_CLIENT)/libbeanstalk.a
	$(CC) $(CFLAGS) -I$(BEANSTALK_CLIENT) bsc.c $(BEANSTALK_CLIENT)/libbeanstalk.a -o bsc

all:	bsc

install:	all
	strip -s bsc
	mkdir -p "$(PREFIX)/bin/"
	cp bsc "$(PREFIX)/bin/"

clean:
	rm bsc
