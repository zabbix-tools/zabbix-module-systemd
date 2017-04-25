# zabbix-module-systemd

zabbix-module-systemd is a loadable Zabbix module that enables Zabbix to query
the systemd D-Bus API for native and granular service monitoring.

This project is a work in progress.

## Download

The following packages are available:

- [Sources](http://s3.cavaliercoder.com/zabbix-contrib/release/zabbix-module-systemd-1.1.0.tar.gz)
- [Zabbix 3.2 - EL7 x86_64 RPM](http://s3.cavaliercoder.com/zabbix-contrib/rhel/7/x86_64/zabbix-module-systemd-1.1.0-1.x86_64.rpm)

## Install

```bash
$ ./configure --with-zabbix=/usr/src/zabbix-3.2.5
$ make
$ sudo make prefix=/usr sysconfdir=/etc libdir=/usr/lib64 install
```

## Keys

```
systemd[<property>]                         return the given property of the
                                            systemd Manager interface

systemd.unit[unit,<interface>,<property>]   return the given property of the
                                            given interface of the given unit
                                            name

systemd.unit.discovery[]                    discovery all known units

systemd.service.info[service,<param>]       query various service stats, similar
                                            to service.info on the Windows agent

systemd.service.discovery[]                 discovery all known services

systemd.modver[]                            version of the loaded module
```

For a list of available unit interfaces and properties, see the
[D-Bus API of systemd/PID 1](https://www.freedesktop.org/wiki/Software/systemd/dbus).

## SELinux

If you have configure SELinux in enforcing mode, you might see the following
error in your Zabbix logs, when attempting to use item keys from this module:

```
[systemd] org.freedesktop.DBus.Error.AccessDenied: SELinux policy denies access
```

This is because the SELinux policy that ships with RedHat/CentOS does not
explicitely allow the Zabbix agent to communicate with D-Bus. This package
includes an extension module to grant Zabbix only the permissions it requires
for read-only access.

After installing this package, the SELinux module can be enabled by running:

```bash
$ semodule -v -i /usr/share/selinux/packages/zabbix-module-systemd/libzbxsystemd.pp
```
