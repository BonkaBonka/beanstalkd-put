#

CFLAGS?=-Wall -pedantic -Os -g -std=c99
PREFIX?=/usr/local

BEANSTALK_CLIENT?=beanstalk-client
YAML?=libyaml

all:	bsc

$(BEANSTALK_CLIENT)/beanstalk.h:
	git submodule init
	git submodule update

$(BEANSTALK_CLIENT)/libbeanstalk.a: $(BEANSTALK_CLIENT)/beanstalk.h
	sh -cx "cd $(BEANSTALK_CLIENT); make -f makefile libbeanstalk.a"

$(YAML)/yaml.h:
	hg clone -r 0.1.5 https://bitbucket.org/xi/libyaml $(YAML)
	ln -s include/yaml.h $(YAML)

$(YAML)/libyaml.a: $(YAML)/yaml.h
	sh -cx "cd $(YAML); ./bootstrap; ./configure --enable-static; make"
	ln -s src/.libs/libyaml.a $(YAML)

bsc: bsc.c $(BEANSTALK_CLIENT)/libbeanstalk.a $(YAML)/libyaml.a
	$(CC) $(CFLAGS) -I$(BEANSTALK_CLIENT) -I$(YAML) bsc.c $(BEANSTALK_CLIENT)/libbeanstalk.a $(YAML)/libyaml.a -o bsc

install:	all
	strip -s bsc
	mkdir -p "$(PREFIX)/bin/"
	cp bsc "$(PREFIX)/bin/"

clean:
	rm bsc
