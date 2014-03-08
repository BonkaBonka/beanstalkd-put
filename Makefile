#

CFLAGS?=-Wall -pedantic -Os -g -std=c99
PREFIX?=/usr/local

BEANSTALK_CLIENT?=beanstalk-client

all:	bsc

$(BEANSTALK_CLIENT)/beanstalk.h:
	git submodule init
	git submodule update

$(BEANSTALK_CLIENT)/libbeanstalk.a: $(BEANSTALK_CLIENT)/beanstalk.h
	sh -cx "cd $(BEANSTALK_CLIENT); make -f makefile libbeanstalk.a"

bsc: bsc.c $(BEANSTALK_CLIENT)/libbeanstalk.a
	$(CC) $(CFLAGS) -I$(BEANSTALK_CLIENT) bsc.c $(BEANSTALK_CLIENT)/libbeanstalk.a -o bsc

install:	all
	strip -s bsc
	mkdir -p "$(PREFIX)/bin/"
	cp bsc "$(PREFIX)/bin/"

clean:
	rm bsc
