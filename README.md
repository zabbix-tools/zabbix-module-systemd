# zabbix-module-systemd

zabbix-module-systemd is a loadable Zabbix module that enables Zabbix to query
the systemd D-Bus API for native and granular service monitoring.

This project is a work in progress.

## Download

The following packages are available:

- [Sources](http://s3.cavaliercoder.com/zabbix-contrib/release/zabbix-module-systemd-1.0.0.tar.gz)
- [Zabbix 3.2 - EL7 x86_64 RPM](http://s3.cavaliercoder.com/zabbix-contrib/rhel/7/x86_64/zabbix-module-systemd-1.0.0-1.x86_64.rpm)

## Install

```bash
$ ./configure --with-zabbix=/usr/src/zabbix-3.2.4
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

systemd.modver[]                            version of the loaded module

systemd.service.info[service,<param>]       query various service stats, similar
                                            to service.info on the Windows agent
```

For a list of available unit interfaces and properties, see the
[D-Bus API of systemd/PID 1](https://www.freedesktop.org/wiki/Software/systemd/dbus).
