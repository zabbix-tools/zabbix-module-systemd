#include "libzbxsystemd.h"

// global dbus connection
DBusConnection *conn = NULL;

/*
 * systemd_connect establishes a connection to the d-bus system bus and confirms
 * that the system service is available.
 *
 * Returns FAIL on error.
 */
int systemd_connect()
{
  DBusError err;  
  dbus_error_init(&err);

  // connect to system bus
  conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
  if (dbus_error_is_set(&err)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "failed to get d-bus session: %s",
      err.message);
    dbus_error_free(&err);
    return FAIL;
  }

  zabbix_log(LOG_LEVEL_DEBUG, LOG_PREFIX "connected to d-bus with unique name: %s",
    dbus_bus_get_unique_name(conn));

  // TODO: check for SYSTEMD_SERVICE
  
  return SUCCEED;
}

/*
 * systemd_get_unit fills the given buffer with the Object Path of the given
 * unit name (e.g. sshd.service).
 *
 * If no unit extension is given, .service is appended on behalf of the caller.
 * 
 * Returns FAIL on error.
 */
int systemd_get_unit(char *s, size_t n, const char* unit)
{
  DBusMessage     *msg = NULL;
  DBusMessageIter args;
  const char      *val = NULL;
  char            buf[255], *c = NULL;
  int             type = 0;

  // qualify unit name if no extension given
  zbx_strlcpy(buf, unit, sizeof(buf));
  for (c = &buf[0]; *c; c++)
    if ('.' == *c)
      break;
  
  if ('\0' == *c)
    zbx_strlcpy(c, ".service", 9);
  c = &buf[0];

  // create method call
  msg = dbus_message_new_method_call(
    SYSTEMD_SERVICE_NAME,
    SYSTEMD_ROOT_NODE,
    SYSTEMD_MANAGER_INTERFACE,
    "GetUnit");
  
  dbus_message_iter_init_append(msg, &args);
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &c))
    return FAIL;

  if (NULL == (msg = dbus_exchange_message(msg)))
    return FAIL;

  // read value
  if (!dbus_message_iter_init(msg, &args)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "message has no arguments");
    return FAIL;
  }
  
  if (DBUS_TYPE_OBJECT_PATH != (type = dbus_message_iter_get_arg_type(&args))) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "argument is not an object path: %c", type);
    return FAIL;
  }
  
  dbus_message_iter_get_basic(&args, &val);
  zbx_strlcpy(s, val, n); // WARNING: object paths are unlimited
  dbus_message_unref(msg);

  return SUCCEED;
}

/*
 * systemd_get_service_path fills the given buffer with the first segment of the
 * Service.ExecStart value for the given service object
 */
int systemd_get_service_path(char *s, size_t n, const char *path)
{
  DBusMessageIter *val;
  DBusMessageIter arr, obj;
  char *v = NULL;

  val = dbus_get_property(
                      SYSTEMD_SERVICE_NAME,
                      path,
                      SYSTEMD_SERVICE_INTERFACE,
                      "ExecStart");
  
  if (NULL == val)
    return FAIL;

  // type: a(sasbttuii)
  dbus_message_iter_recurse(val, &arr);   // -> array
  dbus_message_iter_recurse(&arr, &obj);  // -> struct
  dbus_message_iter_get_basic(&obj, &v);  // string

  if (NULL == v)
    return FAIL;

  zbx_strlcpy(s, v, n);
  return SUCCEED;
}

/*
 * systemd_unit_is_service returns non-zero if the given object path is a
 * systemd service unit.
 */
int systemd_unit_is_service(const char *path)
{
  // service objects paths are unbounded in length and end in '_2eservice'
  int len = strnlen(path, 4096);
  const char *ext = path + MAX(0, len - 10);
  return (0 == strncmp(ext, "_2eservice", 11));  
}

/*
 * systemd_service_state_code returns the status code for the given systemd
 * service ActiveState property value or -1 if unknown.
 */
int systemd_service_state_code(const char *state)
{
  // Map systemd ActiveStates to status codes
  // Code definitions aim to roughly balance LSB initscript codes and the Zabbix
  // agent for Windows service status codes.
  // https://www.zabbix.com/documentation/3.2/manual/config/items/itemtypes/zabbix_agent/win_keys
  const int codes[] = { 0, 2, 5, 6, 8 };
  const char *states[] = {
    "active", "activating", "deactivating", "inactive", "failed",
    NULL
  };

  for(int i = 0; states[i]; i++)
    if (0 == strncmp(state, states[i], 13))
      return codes[i];
  
  return -1;
}

/*
 * systemd_service_startup_code returns the startup code for the given system
 * service UnitFileState property value or -1 if unknown.
 */
int systemd_service_startup_code(const char *state)
{
  // Map systemd UnitFileStates to Zabbix service startup codes.
  // Code definitions mimic the Windows service startup codes.
  // https://www.zabbix.com/documentation/3.2/manual/config/items/itemtypes/zabbix_agent/win_keys
  const int codes[] = { 0, 2, 0, 2, 3, 3, 0, 2, 4 };
  const char *states[] = {
    "enabled", "enabled-runtime", "linked", "linked-runtime", "masked",
    "masked-runtime", "static", "disabled", "invalid",
    NULL
  };

  for(int i = 0; states[i]; i++)
    if (0 == strncmp(state, states[i], 13))
      return codes[i];
  
  return -1;
}
