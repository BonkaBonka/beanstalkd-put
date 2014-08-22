#

CFLAGS?=-Wall -pedantic -Os -g -std=c99
PREFIX?=/usr/local

BEANSTALK_CLIENT?=beanstalk-client

BINS?=beanstalkd-put beanstalkd-kick
MANS?=beanstalkd-put.1 beanstalkd-kick.1

%: %.c $(BEANSTALK_CLIENT)/libbeanstalk.a
	$(CC) $(CFLAGS) -I$(BEANSTALK_CLIENT) $^ -o $@

%.1: %.1.ronn
	ronn --organization="beanstalkd-put 1.0" --warnings --roff $<

all:	$(BINS)

$(BEANSTALK_CLIENT)/beanstalk.h:
	git submodule init
	git submodule update

$(BEANSTALK_CLIENT)/libbeanstalk.a: $(BEANSTALK_CLIENT)/beanstalk.h
	sh -cx "cd $(BEANSTALK_CLIENT); make -f makefile libbeanstalk.a"

install:	all
	strip -s $(BINS)
	mkdir -p "$(PREFIX)/bin/"
	cp $(BINS) "$(PREFIX)/bin/"
	mkdir -p "$(PREFIX)/share/man/man1/"
	cp beanstalkd-put.1 "$(PREFIX)/share/man/man1/"

clean:
	rm -f $(BINS) $(MANS)
