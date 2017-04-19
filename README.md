# zabbix-module-systemd

zabbix-module-systemd is a loadable Zabbix module that enables Zabbix to query
the systemd D-Bus API for native and granular service monitoring.

This project is a work in progress.

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

systemd.unit[<unit>,<interface>,<property>] return the given property of the
                                            given interface of the given unit
                                            name

systemd.unit.discovery[]                    discovery all known units

systemd.modver[]                            version of the loaded module
```

For a list of available unit interfaces and properties, see the
[D-Bus API of systemd/PID 1](https://www.freedesktop.org/wiki/Software/systemd/dbus).
