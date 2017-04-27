lib_LTLIBRARIES = libzbxsystemd.la

libzbxsystemd_la_SOURCES = \
	libzbxsystemd.h \
	libzbxsystemd.c \
	systemd.c \
	dbus.c \
	sb.c \
	sb.h

libzbxsystemd_la_CFLAGS = \
	$(DBUS_CPPFLAGS) \
	$(ZABBIX_CPPFLAGS)

libzbxsystemd_la_LDFLAGS = \
	-shared \
	-module \
	-avoid-version \
	$(DBUS_LDFLAGS)

install-data-hook: install-module install-module-conf

# move module into correct location
install-module:
	rm -f \
		$(DESTDIR)$(libdir)/libzbxsystemd.la \
		$(DESTDIR)$(libdir)/libzbxsystemd.so
	$(MKDIR_P) $(DESTDIR)$(libdir)/zabbix/modules/
	$(INSTALL) -m 644 ./.libs/libzbxsystemd.so \
		$(DESTDIR)$(libdir)/zabbix/modules/libzbxsystemd.so
	
# install config file
install-module-conf:
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)/zabbix/zabbix_agentd.d
	test -f "$(DESTDIR)$(sysconfdir)/zabbix/zabbix_agentd.d/libzbxsystemd.conf" || \
		$(INSTALL) -m 0644 ../../../conf/libzbxsystemd.conf \
			$(DESTDIR)$(sysconfdir)/zabbix/zabbix_agentd.d/libzbxsystemd.conf
