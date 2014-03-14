Summary: Simple CLI tool to put jobs into beanstalkd tubes.
Name: beanstalkd-put
Version: 1.0
Release: 1
Group: Applications/Communications
License: BSD-type
Packager: James Cook <bonkabonka@gmail.com>
URL: https://github.com/BonkaBonka/beanstalkd-put
Source: https://github.com/BonkaBonka/beanstalkd-put/archive/v%{version}.tar.gz
BuildRequires: git

%description
beanstalkd-put is a simple CLI utility to insert jobs into beanstalkd
tubes.  The job body can either be specified on the command line or it
can be read from stdin.

%prep
%setup -q

%build
make

%install
PREFIX=$RPM_BUILD_ROOT/usr make install

%clean

%files
%defattr(-,bin,bin)
%{_bindir}/*
%doc %_mandir/man1/*
