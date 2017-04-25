# zabbix-module-systemd

zabbix-module-systemd is a loadable Zabbix module that enables Zabbix to query
the systemd D-Bus API for native and granular system state monitoring.

## Download

The following packages are available:

- [Sources](http://s3.cavaliercoder.com/zabbix-contrib/release/zabbix-module-systemd-1.1.0.tar.gz)
- [Zabbix 3.2 - EL7 x86_64 RPM](http://s3.cavaliercoder.com/zabbix-contrib/rhel/7/x86_64/zabbix-module-systemd-1.1.0-1.x86_64.rpm)

## Install

```bash
$ ./configure --with-zabbix=/usr/src/zabbix-3.2.5
$ make
$ sudo make install
```

If you are using a packaged version of Zabbix, you may with to redirect the
installation directories as follows:

```
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

## Examples

```bash
# return the system architecture
$ zabbix_get -k systemd[Architecture]
x86-64

# discover all units
$ zabbix_get -k systemd.unit.discovery
{
  "data": [
    {
      "{#UNIT.NAME}": "dev-disk-by\\x2dpath-pci\\x2d0000:00:01.1\\x2data\\x2d1.0\\x2dpart2.device",
      "{#UNIT.DESCRIPTION}": "VBOX_HARDDISK 2",
      "{#UNIT.LOADSTATE}": "loaded",
      "{#UNIT.ACTIVESTATE}": "active",
      "{#UNIT.SUBSTATE}": "plugged",
      "{#UNIT.OBJECTPATH}": "/org/freedesktop/systemd1/unit/dev_2ddisk_2dby_5cx2dpath_2dpci_5cx2d0000_3a00_3a01_2e1_5cx2data_5cx2d1_2e0_5cx2dpart2_2edevice",
      "{#UNIT.FOLLOWING}": "sys-devices-pci0000:00-0000:00:01.1-ata1-host0-target0:0:0-0:0:0:0-block-sda-sda2.device"
    },
    {
      "{#UNIT.NAME}": "getty@tty1.service",
      "{#UNIT.DESCRIPTION}": "Getty on tty1",
      "{#UNIT.LOADSTATE}": "loaded",
      "{#UNIT.ACTIVESTATE}": "active",
      "{#UNIT.SUBSTATE}": "running",
      "{#UNIT.OBJECTPATH}": "/org/freedesktop/systemd1/unit/getty_40tty1_2eservice",
      "{#UNIT.FRAGMENTPATH}": "/usr/lib/systemd/system/getty@.service",
      "{#UNIT.UNITFILESTATE}": "enabled"
    },
    ...
  ]
}

# return the location of a mount unit
$ zabbix_get -k systemd.unit[dev-mqueue.mount,Mount,Where]
/dev/mqueue

# return the number of open connections on a socket unit
$ zabbix_get -k systemd.unit[dbus.socket,Socket,NConnections]
1

# discovery all services
$ zabbix_get -k systemd.service.discovery
{
  "data": [
    {
      "{#SERVICE.TYPE}": "service",
      "{#SERVICE.NAME}": "getty@tty1.service",
      "{#SERVICE.DISPLAYNAME}": "Getty on tty1",
      "{#SERVICE.PATH}": "/usr/lib/systemd/system/getty@.service",
      "{#SERVICE.STARTUPNAME}": "enabled"
    },
    {
      "{#SERVICE.TYPE}": "service",
      "{#SERVICE.NAME}": "systemd-fsck-root.service",
      "{#SERVICE.DISPLAYNAME}": "File System Check on Root Device",
      "{#SERVICE.PATH}": "/usr/lib/systemd/system/systemd-fsck-root.service",
      "{#SERVICE.STARTUPNAME}": "static"
    },
    ...
  ]
}

# return the state of a service as an integer
$ zabbix_get -k systemd.service.info[sshd]
0

# return the startup state of a service as an integer
$ zabbix_get -k systemd.service.info[sshd,startup]
0

```



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
