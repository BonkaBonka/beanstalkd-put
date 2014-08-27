## Building a distribution package

### Debian

	git clone https://github.com/BonkaBonka/beanstalkd-put.git
	cd beanstalkd-put
	dpkg-buildpackage -b

### CentOS

NOTE: This is a terrible, awful way to make this go but [*shikata ga nai*](https://en.wikipedia.org/wiki/Shikata_ga_nai)[^1].

	git clone https://github.com/BonkaBonka/beanstalkd-put.git beanstalkd-put-2.0
	cd beanstalkd-put-2.0
	git submodule init
	git submodule update
	cd ..
	tar zcf v2.0.tar.gz beanstalkd-put-2.0

	rpmbuild -tb v2.0.tar.gz

### Arch Linux

	curl -Lo PKGBUILD https://raw.github.com/BonkaBonka/beanstalkd-put/master/arch/PKGBUILD
	makepkg --skipchecksums

[^1]: GitHub currently doesn't include submodules in their release tarballs (Argh!) and `rpmbuild` requires a source archive - it won't directly use a VCS checkout (Blast!) though there are some rather hackish instructions on the Mandriva Community Wiki [on how to work around the issue](http://wiki.mandriva.com/en/Rpmbuild_and_git#First-time_build_for_a_project).