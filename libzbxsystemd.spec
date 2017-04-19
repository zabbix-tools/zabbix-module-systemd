Name        : libzbxsystemd
Vendor      : cavaliercoder
Version     : 1.0.0
Release     : 1
Summary     : systemd monitoring module for Zabbix

Group       : Applications/Internet
License     : GNU GPLv2
URL         : https://github.com/cavaliercoder/libzbxsystemd

Source0     : %{name}-%{version}.tar.gz
Buildroot   : %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires    : zabbix-agent >= 3.2.0
%define moddir %{_libdir}/zabbix

%description
libzbxsystemd is a systemd discovery and monitoring module for the Zabbix
monitoring agent written in C.

%prep
%setup0 -q -n %{name}-%{version}

# fix up some lib64 issues
sed -i.orig -e 's|_LIBDIR=/usr/lib|_LIBDIR=%{_libdir}|g' configure

%build
%configure --enable-dependency-tracking
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

# Move lib into .../modules/
install -dm 755 $RPM_BUILD_ROOT%{moddir}
install -dm 755 $RPM_BUILD_ROOT%{moddir}/modules
mv $RPM_BUILD_ROOT%{_libdir}/%{name}.so $RPM_BUILD_ROOT%{moddir}/modules/%{name}.so

# Create agent config file
install -dm 755 $RPM_BUILD_ROOT%{_sysconfdir}/zabbix/zabbix_agentd.d
install -m 644 zabbix/zabbix_agentd.d/%{name}.conf $RPM_BUILD_ROOT%{_sysconfdir}/zabbix/zabbix_agentd.d/%{name}.conf
install -dm 755 $RPM_BUILD_ROOT%{_sysconfdir}/%{name}.d

%clean
rm -rf $RPM_BUILD_ROOT

%files
%{moddir}/modules/libzbxsystemd.so
%{_sysconfdir}/zabbix/zabbix_agentd.d/%{name}.conf

%changelog
* Wed Apr 19 2017 Ryan Armstrong <ryan@cavaliercoder.com> 0.1.0-1
- Initial release