#

CFLAGS?=-Wall -pedantic -Os -g -std=c99
PREFIX?=/usr/local

BEANSTALK_CLIENT?=beanstalk-client

all:	beanstalkd-put

$(BEANSTALK_CLIENT)/beanstalk.h:
	git submodule init
	git submodule update

$(BEANSTALK_CLIENT)/libbeanstalk.a: $(BEANSTALK_CLIENT)/beanstalk.h
	sh -cx "cd $(BEANSTALK_CLIENT); make -f makefile libbeanstalk.a"

beanstalkd-put: beanstalkd-put.c $(BEANSTALK_CLIENT)/libbeanstalk.a
	$(CC) $(CFLAGS) -I$(BEANSTALK_CLIENT) beanstalkd-put.c $(BEANSTALK_CLIENT)/libbeanstalk.a -o beanstalkd-put

beanstalkd-put.1: beanstalkd-put.1.ronn
	ronn --organization="beanstalkd-put 1.0" --warnings --roff $<

install:	all
	strip -s beanstalkd-put
	mkdir -p "$(PREFIX)/bin/"
	cp beanstalkd-put "$(PREFIX)/bin/"
	mkdir -p "$(PREFIX)/share/man/man1/"
	cp beanstalkd-put.1 "$(PREFIX)/share/man/man1/"

clean:
	rm beanstalkd-put
