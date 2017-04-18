lib_LTLIBRARIES = libzbxsystemd.la

libzbxsystemd_la_SOURCES = \
	libzbxsystemd.h \
	libzbxsystemd.c \
	systemd.c \
	dbus.c

libzbxsystemd_la_CFLAGS = \
	$(DEBUG_CPPFLAGS) \
	$(DBUS_CPPFLAGS) \
	$(ZABBIX_CPPFLAGS) \
	$(libzbxsystemd_CPPFLAGS)

libzbxsystemd_la_LDFLAGS = \
	-shared \
	-module \
	-avoid-version \
	$(DBUS_LDFLAGS) \
	$(libzbxsystemd_LIBS)

# install module into correct location
install-exec-hook: install-agent-config
	mkdir -p $(DESTDIR)$(libdir)/zabbix/modules/
	rm -f $(DESTDIR)$(libdir)/libzbxsystemd.la
	mv -f $(DESTDIR)$(libdir)/libzbxsystemd.so \
		$(DESTDIR)$(libdir)/zabbix/modules/libzbxsystemd.so

# install module configuration file
install-agent-config:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)/zabbix/zabbix_agentd.d
	echo "LoadModule=libzbxsystemd.so" > \
		$(DESTDIR)$(sysconfdir)/zabbix/zabbix_agentd.d/libzbxsystemd.conf
