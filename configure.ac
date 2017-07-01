AC_INIT([zabbix-module-systemd], [1.2.0], [ryan@cavaliercoder.com])
AM_INIT_AUTOMAKE([-Wall -Werror -Wno-portability foreign])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

# init libtool
# need > v1.4 to drop 'lib' prefix from linker output
LT_PREREQ([1.4.0]) 
LT_INIT

AC_PROG_CC
AC_PROG_CC_STDC
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

# check for stdlib header files
AC_HEADER_STDC

# Checking for Zabbix headers
AX_CHECK_ZABBIX
if test ! "x$found_zabbix" = "xyes"; then
  AC_MSG_ERROR([Zabbix headers not found])
fi

# DBUS flags
AC_MSG_CHECKING([for D-Bus libraries])
DBUS_VERSION="`pkg-config dbus-1 --modversion 2>/dev/null`"

if test -z "$DBUS_VERSION"; then
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([D-Bus library not found])
else
  DBUS_CPPFLAGS="`pkg-config dbus-1 --cflags`"
  DBUS_LDFLAGS="`pkg-config dbus-1 --libs`"

  AC_SUBST([DBUS_VERSION])
  AC_SUBST([DBUS_CPPFLAGS])
  AC_SUBST([DBUS_LDFLAGS])

  AC_MSG_RESULT([$DBUS_VERSION])
  AC_DEFINE([HAVE_DBUS], [1], [Define to 1 if D-Bus libraries are available])
fi

dnl
dnl Enable selinux module
dnl
AC_ARG_ENABLE(semodule,[  --enable-semodule       build SELinux module],
[case "${enableval}" in
  yes) semodule=yes ;;
  no)  semodule=no ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-semodule]) ;;
esac],
[semodule=no])
AM_CONDITIONAL(SEMODULE, test "x$semodule" = "xyes")

dnl
dnl output
dnl
AC_CONFIG_FILES([
 Makefile
 src/modules/systemd/Makefile
 selinux/Makefile
])
AC_OUTPUT

echo "
Configuration:

  Detected OS:           ${host_os}
  Module install path:   ${libdir}

  Compiler:              ${CC}
  Compiler flags:        ${CFLAGS}

  Library-specific flags:"

if test "x$ZABBIX_CPPFLAGS" != "x"; then
  echo "    zabbix:              ${ZABBIX_CPPFLAGS}"
fi

if test "x$DBUS_CPPFLAGS" != "x"; then
  echo "    d-bus:               ${DBUS_CPPFLAGS}"
fi

echo "
  Enable SELinux module: ${semodule}"

echo
echo "************************************************************"
echo "*                 Now run '${am_make} install'                   *"
echo "*                                                          *"
echo "*            Thank you for using libzbxsystemd!            *"
echo "*               <http://cavaliercoder.com>                 *"
echo "************************************************************"
echo
