Name:             ipvshttpd
Version:          0.0.3
Release:          1%{?dist}
Summary:          IPVS REST api http daemon

Group:            Applications/Network
License:          GPLv3
URL:              https://github.com/jnse/ipvshttpd
Source0:          ipvshttpd.tar.gz

BuildRoot:        %{_tmppath}/ipvshttpd-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:    jsoncpp-devel
BuildRequires:    scl-utils
BuildRequires:    scl-utils-build
BuildRequires:    devtoolset-3-gcc
BuildRequires:    devtoolset-3-gcc-c++
BuildRequires:    cmake3

Requires:         boost 
Requires(post):   chkconfig
Requires(preun):  chkconfig
Requires(preun):  initscripts
Requires(postun): initscripts 

%description
ipvshttpd is a daemon serving up a rest api 
for interacting with the Linux ipvs stack.

%prep
%setup -q -n ipvshttpd

%build
pushd depend/cpp-netlib
rm -fr build
mkdir -p build > /dev/null
cd build
scl enable devtoolset-3 -- cmake3 -DCPP-NETLIB_BUILD_TESTS=FALSE ..
scl enable devtoolset-3 -- make
popd > /dev/null
rm -fr build
mkdir -p build
pushd build
scl enable devtoolset-3 -- cmake3 ..
scl enable devtoolset-3 -- make

%install
rm -rf %{buildroot}
install -p -D -m 755 build/ipvshttpd %{buildroot}%{_sbindir}/ipvshttpd
install -p -D -m 755 dist/ipvshttpd.init %{buildroot}%{_initddir}/ipvshttpd
install -p -D -m 644 doc/ipvshttpd-example.conf %{buildroot}doc/ipvshttpd-example.conf

%post
/sbin/chkconfig --add ipvshttpd
mkdir -p /usr/share/ipvshttpd
openssl dhparam -out /usr/share/ipvshttpd/dh1024.pem 1024

%preun
if [ $1 -eq 0 ] ; then
  /sbin/service ipvshttpd stop >/dev/null 2>&1
  /sbin/chkconfig --del ipvshttpd
fi
 
%files
%defattr(-,root,root,-)
%attr(0755,root,root) %{_sbindir}/ipvshttpd
%attr(0755,root,root) %{_initddir}/ipvshttpd
%doc doc/ipvshttpd-example.conf

%changelog
* Sat May 26 2018 John Sennesael <john@adminking.com> - 0.0.3-1
- Initial release.

