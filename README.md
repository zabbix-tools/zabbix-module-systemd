# libzbxsystemd

Native systemd monitoring module for Zabbix.

This project is a work in progress.

## Install

```bash
$ ./configure --with-zabbix=/usr/src/zabbix-3.2.4
$ make
$ make sysconfdir=/etc libdir=/usr/lib64 install
```

## Keys

```
systemd.unit[<unit>,<property>]   return the given property of the given unit
                                  property is case sensitive and defaults to
                                  'SubState'

systemd.unit.discovery[]          discovery all known units

systemd.modver[]                  version of the loaded module
```

For a list of available unit properties, see the
[D-Bus API of systemd/PID 1](https://www.freedesktop.org/wiki/Software/systemd/dbus#unitobjects).

## License

Copyright (c) 2017 Ryan Armstrong

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
