ACLOCAL_AMFLAGS = -Im4

if SEMODULE
SEMODULE_SUBDIRS = \
	selinux
endif

SUBDIRS = \
	src/modules/systemd \
	$(SEMODULE_SUBDIRS)

EXTRA_DIST = \
	README.md \
	COPYING \
	conf/libzbxsystemd.conf

install-data-hook:
	$(INSTALL) -d $(DESTDIR)$(docdir)-$(PACKAGE_VERSION)
	$(INSTALL) -m 0644 README.md $(DESTDIR)$(docdir)-$(PACKAGE_VERSION)/README.md
	$(INSTALL) -m 0644 COPYING $(DESTDIR)$(docdir)-$(PACKAGE_VERSION)/COPYING

rpm: dist
	mkdir -p ~/rpmbuild/SPECS
	mkdir -p ~/rpmbuild/SOURCES
	install -m 644 $(PACKAGE_NAME).spec ~/rpmbuild/SPECS/$(PACKAGE_NAME).spec
	install -m 644 \
		$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz \
		~/rpmbuild/SOURCES/$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
	rpmbuild -ba ~/rpmbuild/SPECS/$(PACKAGE_NAME).spec

check-local: all
	zabbix_agent_bench \
		-keys=bench.keys \
		-iterations=1 \
		-strict=true
