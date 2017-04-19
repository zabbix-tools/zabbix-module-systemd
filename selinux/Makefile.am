all: libzbxsystemd.pp

libzbxsystemd.mod: libzbxsystemd.te
	checkmodule -m -M libzbxsystemd.te -o libzbxsystemd.mod

libzbxsystemd.pp: libzbxsystemd.mod
	semodule_package -m libzbxsystemd.mod -o libzbxsystemd.pp

clean-local:
	rm -vf libzbxsystemd.mod libzbxsystemd.pp

install-data-local: libzbxsystemd.pp
	semodule -v -i libzbxsystemd.pp

# .PHONY: all clean install

EXTRA_DIST = libzbxsystemd.te
