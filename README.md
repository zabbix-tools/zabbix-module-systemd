# Native Zabbix systemd monitoring [![Build Status](https://travis-ci.org/cavaliercoder/zabbix-module-systemd.svg?branch=master)](https://travis-ci.org/cavaliercoder/zabbix-module-systemd)

Zabbix module that enables Zabbix to query the systemd D-Bus API for native and
granular system state monitoring + relative cgroup (CPU, MEM, IO, ...) metrics.

## Download

The following packages are available:

- [Sources](http://s3.cavaliercoder.com/zabbix-contrib/release/zabbix-module-systemd-1.2.0.tar.gz)
- [Zabbix 3.2 - EL7 x86_64 RPM](http://s3.cavaliercoder.com/zabbix-contrib/rhel/7/x86_64/zabbix-module-systemd-1.2.0-1.x86_64.rpm)

## Install

```bash
# Required CentOS/RHEL apps:   yum install -y libtool make autoconf automake dbus-devel
# Required Debian/Ubuntu apps: apt-get install -y libtool make autoconf automake libdbus-1-dev
# Required Fedora apps:        dnf install -y libtool make autoconf automake dbus-devel
# Required openSUSE apps:      zypper install -y libtool make autoconf automake dbus-1-devel gcc
# Required Gentoo apps 1:      emerge sys-devel/libtool sys-devel/make sys-devel/autoconf
# Required Gentoo apps 2:      emerge sys-devel/automake sys-devel/gcc
# Source, use your version:    
# git clone https://git.zabbix.com/scm/zbx/zabbix.git --depth 1 --single-branch --branch 4.2.6 /usr/src/zabbix
# Generate config.h: bash -c "cd /usr/src/zabbix; ./bootstrap.sh; ./configure"
./autogen.sh
./configure --with-zabbix=/usr/src/zabbix
make
sudo make install
```

If you are using a packaged version of Zabbix, you may wish to redirect the
installation directories as follows:

```bash
sudo make prefix=/usr sysconfdir=/etc libdir=/usr/lib64 install
```

[Configure Zabbix agent to load module](https://www.zabbix.com/documentation/3.4/manual/config/items/loadablemodules)
`libzbxsystemd.so`.

## Available keys

Note: `systemd.cgroup.*` keys require the cgroup accounting. The system default
for this setting may be controlled with `Default*Accounting` settings in
[systemd-system.conf](https://www.freedesktop.org/software/systemd/man/systemd-system.conf.html).

Example how to enable cgroup accounting for Zabbix systemd monitoring:

```bash
sed -i -e "s/.*DefaultCPUAccounting=.*/DefaultCPUAccounting=yes/g" /etc/systemd/system.conf
sed -i -e "s/.*DefaultBlockIOAccounting=.*/DefaultBlockIOAccounting=yes/g" /etc/systemd/system.conf
sed -i -e "s/.*DefaultMemoryAccounting=.*/DefaultMemoryAccounting=yes/g" /etc/systemd/system.conf
systemctl daemon-reexec
systemctl restart zabbix-agent
```

| Key | Description |
| ------------------------------ | ----------- |
| **systemd[\<property\>]** | Return the given property of the systemd Manager interface. |
| **systemd.unit[unit,\<interface\>,\<property\>]** | Return the given property of the given interface of the given system unit name. For a list of available unit interfaces and properties, see the [D-Bus API of systemd/PID 1](https://www.freedesktop.org/wiki/Software/systemd/dbus) or [Debugging](#debugging) |
| **systemd.unit.discovery[\<type\>]** | Discovery all known system units of the given type (default: `all`). |
| **systemd.service.info[service,\<param\>]** | Query various system service stats (state, displayname, path, user, startup, description), similar to `service.info` on the Windows agent. |
| **systemd.service.discovery[]** | Discovery all known system services. |
| **systemd.cgroup.cpu[\<unit\>,\<cmetric\>]** | **CPU metrics:**<br>**cmetric** - any available CPU metric in the pseudo-file cpuacct.stat/cpu.stat, e.g.: *system, user, total (current sum of system/user* or cgroup [throttling metrics](https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/6/html/Resource_Management_Guide/sec-cpu.html): *nr_throttled, throttled_time*<br>Note: CPU user/system/total metrics must be recalculated to % utilization value by Zabbix - *Delta (speed per second)*. |
| **systemd.cgroup.dev[\<unit\>,\<bfile\>,\<bmetric\>]** | **Blk IO metrics:**<br>**bfile** - cgroup blkio pseudo-file, e.g.: *blkio.io_merged, blkio.io_queued, blkio.io_service_bytes, blkio.io_serviced, blkio.io_service_time, blkio.io_wait_time, blkio.sectors, blkio.time, blkio.avg_queue_size, blkio.idle_time, blkio.dequeue, ...*<br>**bmetric** - any available blkio metric in selected pseudo-file, e.g.: *Total*. Option for selected block device only is also available e.g. *'8:0 Sync'* (quotes must be used in key parameter in this case)<br>Note: Some pseudo blkio files are available only if kernel config *CONFIG_DEBUG_BLK_CGROUP=y*. |
| **systemd.cgroup.mem[\<unit\>,\<mmetric\>]** | **Memory metrics:**<br>**mmetric** - any available memory metric in the pseudo-file memory.stat, e.g.: *cache, rss, mapped_file, pgpgin, pgpgout, swap, pgfault, pgmajfault, inactive_anon, active_anon, inactive_file, active_file, unevictable, hierarchical_memory_limit, hierarchical_memsw_limit, total_cache, total_rss, total_mapped_file, total_pgpgin, total_pgpgout, total_swap, total_pgfault, total_pgmajfault, total_inactive_anon, total_active_anon, total_inactive_file, total_active_file, total_unevictable*.<br>Note: if you have problem with memory metrics, be sure that memory cgroup subsystem is enabled - kernel parameter: *cgroup_enable=memory* |
| **systemd.modver[]** | Version of the loaded systemd module. |

## Templates

Available examples of monitoring templates:

- [Template App systemd services.xml](https://raw.githubusercontent.com/cavaliercoder/zabbix-module-systemd/master/templates/Template%20App%20systemd%20services.xml) - the discovery of enabled systemd services with True condition result

## Examples

```bash
# return the system architecture
$ zabbix_get -k systemd[Architecture]
x86-64

# discover all units - filtering for sockets
$ zabbix_get -k systemd.unit.discovery[socket]
{
  "data": [
    {
      "{#UNIT.NAME}": "dbus.socket",
      "{#UNIT.DESCRIPTION}": "D-Bus System Message Bus Socket",
      "{#UNIT.LOADSTATE}": "loaded",
      "{#UNIT.ACTIVESTATE}": "active",
      "{#UNIT.SUBSTATE}": "running",
      "{#UNIT.OBJECTPATH}": "/org/freedesktop/systemd1/unit/dbus_2esocket",
      "{#UNIT.FRAGMENTPATH}": "/usr/lib/systemd/system/dbus.socket",
      "{#UNIT.UNITFILESTATE}": "static",
      "{#UNIT.CONDITIONRESULT}": "yes"
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

# discover all services
$ zabbix_get -k systemd.service.discovery[service]
{
  "data": [
    {
      "{#SERVICE.TYPE}": "service",
      "{#SERVICE.NAME}": "dbus.service",
      "{#SERVICE.DISPLAYNAME}": "D-Bus System Message Bus",
      "{#SERVICE.PATH}": "/usr/lib/systemd/system/dbus.service",
      "{#SERVICE.STARTUPNAME}": "static",
      "{#SERVICE.CONDITIONRESULT}": "yes"
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

# total cpu usage of dbus.service
$ zabbix_get -k systemd.cgroup.cpu[dbus.service,total]
44

# resident set size (RSS) mem usage of dbus.service
$ zabbix_get -k systemd.cgroup.mem[dbus.service,rss]
663552

# total queued iops of dbus.service
$ zabbix_get -k systemd.cgroup.dev[dbus.service,blkio.io_queued,Total]
0
```

## Debugging

Please use `systemctl`, `gdbus`, `busctl`, `zabbix_get` utilities for debugging
systemd unit properties and their values. For example debugging of
`ConditionResult` property value for sshd service:

```
$ systemctl show sshd.service | grep ConditionResult
ConditionResult=yes
$ gdbus introspect --system --dest org.freedesktop.systemd1 --object-path \
  /org/freedesktop/systemd1/unit/sshd_2eservice \
  /org/freedesktop/systemd1/unit/sshd_2eservice | grep ConditionResult
      readonly b ConditionResult = true;
$ busctl introspect org.freedesktop.systemd1 /org/freedesktop/systemd1/unit/sshd_2eservice \
  | grep ConditionResult
.ConditionResult                    property  b              true               emits-change
$ zabbix_get -k systemd.unit[sshd.service,Unit,ConditionResult]
1
```

Enable `DebugLevel=5` in the Zabbix agent config to see systemd module debug
output in the zabbix-agent log file.

## SELinux

If you have configured SELinux in enforcing mode, you might see the following
error in your Zabbix logs, when attempting to use item keys from this module:

```text
[systemd] org.freedesktop.DBus.Error.AccessDenied: SELinux policy denies access
```

This is because the SELinux policy that ships with RedHat/CentOS does not
explicitly allow the Zabbix agent to communicate with D-Bus. This package
includes an extension module to grant Zabbix only the permissions it requires
for read-only access.

To build the SELinux module, add `--enable-semodule` to the build configuration:

```bash
./configure --with-zabbix=/usr/src/zabbix-3.2.6 --enable-semodule
make
sudo make prefix=/usr sysconfdir=/etc libdir=/usr/lib64 install
```

After installing this package, the SELinux module `libzbxsystemd.pp` can be
enabled by running:

```bash
semodule -v -i /usr/share/selinux/packages/zabbix-module-systemd/libzbxsystemd.pp
```

## Built-in Zabbix systemd support

Zabbix agent 2 supports systemd from version 4.4+ ([ZBXNEXT-2871](https://support.zabbix.com/browse/ZBXNEXT-2871)).
Keys: `systemd.unit.discovery`, `systemd.unit.info`. However this module still
offers more options for systemd monitoring.

