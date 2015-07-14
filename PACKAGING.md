## Building a distribution package

### Debian

	git clone https://github.com/BonkaBonka/beanstalkd-put.git
	cd beanstalkd-put
	dpkg-buildpackage -b

### CentOS

NOTE: This is a terrible, awful way to make this go but [*shikata ga nai*](https://en.wikipedia.org/wiki/Shikata_ga_nai)[^1].

	git clone --recursive --branch v2.1 --depth 1 git://github.com/BonkaBonka/beanstalkd-put.git beanstalkd-put-2.1
	tar --create --file v2.1.tar.gz --gzip --exclude-vcs beanstalkd-put-2.1

	rpmbuild -tb v2.1.tar.gz

### Arch Linux

	curl -Lo PKGBUILD https://raw.github.com/BonkaBonka/beanstalkd-put/master/arch/PKGBUILD
	makepkg --skipchecksums

[^1]: GitHub currently doesn't include submodules in their release tarballs (Argh!) and `rpmbuild` requires a source archive - it won't directly use a VCS checkout (Blast!) though there are some rather hackish instructions on the Mandriva Community Wiki [on how to work around the issue](https://web.archive.org/web/20130618060523/http://wiki.mandriva.com/en/Rpmbuild_and_git#First-time_build_for_a_project).
