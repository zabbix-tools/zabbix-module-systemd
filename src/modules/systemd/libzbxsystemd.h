#ifndef LIBZBXSVC_H
#define LIBZBXSVC_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

// string builder
#include "sb.h"

// Zabbix source headers
#define HAVE_TIME_H 1
#include <sysinc.h>
#include <module.h>
#include <common.h>
#include <log.h>
#include <zbxjson.h>
#include <version.h>
#include <db.h>

#ifndef MAX
#define MAX(a, b)     ( (a) < (b) ? (b) : (a) )
#endif

// d-bus headers
#include <dbus/dbus.h>

#ifndef ZBX_MODULE_API_VERSION
#define ZBX_MODULE_API_VERSION  ZBX_MODULE_API_VERSION_ONE
#endif

#define LOG_PREFIX              "[systemd] "

// pid that initialised the module, before forking workers.
int mainpid;

// timeout set by host agent
int timeout;

// D-Bus api
#define DBUS_PROPERTIES_INTERFACE     "org.freedesktop.DBus.Properties"

int               dbus_connect();
int               dbus_check_error(DBusMessage*);
int               dbus_message_iter_next_n(DBusMessageIter *iter, int n);
DBusMessage       *dbus_exchange_message(DBusMessage *msg);
DBusMessageIter   *dbus_get_property(
                                const char*,
                                const char*,
                                const char*,
                                const char*);

int dbus_get_property_string(
                char          *s,
                const size_t  n,
                const char    *service,
                const char    *path,
                const char    *interface,
                const char    *property); 

int dbus_get_property_json(
                struct zbx_json *j,
                const char      *key,
                const char      *path,
                const char      *interface,
                const char      *property);

int dbus_marshall_property(
                AGENT_RESULT*,
                const char*,
                const char*,
                const char*,
                const char*);

// systemd api
#define SYSTEMD_SERVICE_NAME          "org.freedesktop.systemd1"
#define SYSTEMD_ROOT_NODE             "/org/freedesktop/systemd1"
#define SYSTEMD_MANAGER_INTERFACE     SYSTEMD_SERVICE_NAME ".Manager"
#define SYSTEMD_UNIT_INTERFACE        SYSTEMD_SERVICE_NAME ".Unit"
#define SYSTEMD_SERVICE_INTERFACE     SYSTEMD_SERVICE_NAME ".Service"

DBusConnection *conn;

int systemd_get_unit(char *s, size_t n, const char* unit);
int systemd_unit_is_service(const char *path);
int systemd_cmptype(const char *unit, const char *type);
int systemd_service_state_code(const char *state);
int systemd_service_startup_code(const char *state);
int systemd_get_service_path(char *s, size_t n, const char *path);

#endif
