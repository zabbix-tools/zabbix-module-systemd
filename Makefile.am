ACLOCAL_AMFLAGS = -Im4

SUBDIRS = \
	src \
	selinux

EXTRA_DIST = \
	README.md

rpm: dist
	mkdir -p ~/rpmbuild/SPECS
	mkdir -p ~/rpmbuild/SOURCES
	install -m 644 $(PACKAGE_NAME).spec ~/rpmbuild/SPECS/$(PACKAGE_NAME).spec
	install -m 644 \
		$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz \
		~/rpmbuild/SOURCES/$(PACKAGE_NAME)-$(PACKAGE_VERSION).tar.gz
	rpmbuild -ba ~/rpmbuild/SPECS/$(PACKAGE_NAME).spec
