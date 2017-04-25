#include "libzbxsystemd.h"

// global dbus connection
DBusConnection *conn = NULL;

/*
 * dbus_connect establishes a connection to the d-bus system bus.
 *
 * Returns FAIL on error.
 */
int dbus_connect()
{
  if (NULL != conn)
    return SUCCEED;

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

  // TODO: check for SYSTEMD_SERVICE?
  
  return SUCCEED;
}

/*
 * dbus_check_error returns FAIL if the given message is an error message. The
 * error detail is logged to the Zabbix log on behalf of the caller.
 */
int dbus_check_error(DBusMessage *msg)
{
  const char *s = "\0";
  DBusMessageIter args;

  if (DBUS_MESSAGE_TYPE_ERROR == dbus_message_get_type(msg)) {
    if (dbus_message_iter_init(msg, &args)) {
      if (DBUS_TYPE_STRING == dbus_message_iter_get_arg_type(&args)) {
        dbus_message_iter_get_basic(&args, &s);
      }
    }
    
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "%s: %s", dbus_message_get_error_name(msg), s);
    dbus_message_unref(msg);
    return FAIL;
  }

  return SUCCEED;
}

/*
 * dbus_message_iter_next_n calls dbus_message_iter_next n times and returns
 * non-zero if it reaches the end of the iterator.
 */
int dbus_message_iter_next_n(DBusMessageIter *iter, int n)
{
  for (int i = 0; i < n; i++)
    if (!dbus_message_iter_next(iter))
      return 0;

  return 1;
}

/*
 * dbus_exchange_message sends the given message and returns the response
 * message or NULL if an error occurs.
 */
DBusMessage *dbus_exchange_message(DBusMessage *msg) {
  DBusPendingCall *pending = NULL;

  if (NULL == msg) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "message is null");
    return NULL;
  }
  
  // send message
  if (!dbus_connection_send_with_reply (conn, msg, &pending, timeout)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "oom sending message");
    return NULL;
  }
  
  if (NULL == pending) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "pending message is null");
    return NULL;
  }

  dbus_connection_flush(conn);
  dbus_message_unref(msg);

  // get reply
  dbus_pending_call_block(pending);
  msg = dbus_pending_call_steal_reply(pending);
  if (NULL == msg) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "returned message is null");
    return NULL;
  }
  dbus_pending_call_unref(pending);

  // check for errors
  if (dbus_check_error(msg))
    return NULL;

  return msg;
}

/*
 * dbus_get_property returns a pointer to an iterator containing the values of
 * the given property or NULL if an error occurs.
 */
DBusMessageIter* dbus_get_property(
  const char *service,
  const char *path,
  const char *interface,
  const char *property
) {
  DBusMessage     *msg = NULL;
  DBusMessageIter args;
  DBusMessageIter *iter = NULL;

  zabbix_log(LOG_LEVEL_DEBUG, 
                    LOG_PREFIX "getting property:\n"
                    "\tservice: %s\n"
                    "\tobject path: %s\n"
                    "\tinterface: %s\n"
                    "\tproperty: %s",
                    service,
                    path,
                    interface,
                    property);

  // create method call
  msg = dbus_message_new_method_call(
    service,
    path,
    DBUS_PROPERTIES_INTERFACE,
    "Get");
  
  if (NULL == msg) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "message is null");
    return NULL;
  }

  dbus_message_iter_init_append(msg, &args);
  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &interface))
    return NULL;

  if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &property))
    return NULL;

  if (NULL == (msg = dbus_exchange_message(msg)))
    return NULL;

  // check type
  if (!dbus_message_iter_init(msg, &args)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "message has no arguments");
    return NULL;
  }
  
  if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(&args)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "argument is not a variant");
    return NULL;
  }
  
  // return value iterator
  iter = zbx_malloc(iter, sizeof(DBusMessageIter));
  dbus_message_iter_recurse(&args, iter);
  dbus_message_unref(msg);

  return iter;
}

/*
 * dbus_get_property_string fills the given buffer with the value of the given
 * object property.
 */
int dbus_get_property_string(
  char          *s,
  const size_t  n,
  const char    *service,
  const char    *path,
  const char    *interface,
  const char    *property
) {
  DBusMessageIter *iter = NULL;
  char            *value = NULL;

  iter = dbus_get_property(
                      service,
                      path,
                      interface,
                      property);
  
  if (NULL == iter)
    return FAIL;
  
  if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(iter))
    return FAIL;
  
  dbus_message_iter_get_basic(iter, &value);
  zbx_strlcpy(s, value, n);
  return SUCCEED;
}

/*
 * dbus_get_property_json appends the value of the given property as a key/val
 * pair to the given discovery json document.
 */
int dbus_get_property_json(
              struct zbx_json *j,
              const char      *key,
              const char      *path,
              const char      *interface,
              const char      *property
) {
  char buf[1024];
  buf[0] = '\0';
  if (SUCCEED == dbus_get_property_string(
    buf,
    sizeof(buf),
    SYSTEMD_SERVICE_NAME,
    path,
    interface,
    property
  )) {
    if ('\0' != buf[0])
      zbx_json_addstring(j, key, buf, ZBX_JSON_TYPE_STRING);
    return SUCCEED;
  }

  return FAIL;
}

/*
 * dbus_marshall_property gets the value of a d-bus property and marshalls it
 * into a Zabbix AGENT_RESULT struct.
 *
 * Returns SYSINFO_RET_FAIL on error.
 */
int dbus_marshall_property(
  AGENT_RESULT  *result,
  const char    *service,
  const char    *path,
  const char    *interface,
  const char    *property
) {
  int             type = 0;
  DBusMessageIter *iter = NULL, arr;
  DBusBasicValue  value;
  char            *s = NULL;
  StringBuilder   *sb = NULL;

  iter = dbus_get_property(
                      service,
                      path,
                      interface,
                      property);

  if (NULL == iter) {
    SET_MSG_RESULT(result, strdup("failed to get property"));
    return SYSINFO_RET_FAIL;
  }
  
  type = dbus_message_iter_get_arg_type(iter);

  // marshal string array
  if (DBUS_TYPE_ARRAY == type) {
    dbus_message_iter_recurse(iter, &arr);
    sb = sb_create();
    while (DBUS_TYPE_INVALID != (type = dbus_message_iter_get_arg_type(&arr))) {
      if (DBUS_TYPE_STRING == type) {
        if (!sb_empty(sb))
          sb_append(sb, ",");
        dbus_message_iter_get_basic(&arr, &s);
        sb_append(sb, s);
      }
      dbus_message_iter_next(&arr);
    }

    SET_STR_RESULT(result, sb_concat(sb));
    sb_free(sb);
    return SYSINFO_RET_OK;
  }

  // marshal basic type
  dbus_message_iter_get_basic(iter, &value);
  switch (type) {
  case DBUS_TYPE_STRING:
    SET_STR_RESULT(result, strdup(value.str));
    return SYSINFO_RET_OK;

  case DBUS_TYPE_BOOLEAN:
    SET_UI64_RESULT(result, value.bool_val);
    return SYSINFO_RET_OK;

  case DBUS_TYPE_UINT64:
  case DBUS_TYPE_INT64:
    SET_UI64_RESULT(result, value.u64);
    return SYSINFO_RET_OK;

  case DBUS_TYPE_UINT32:
  case DBUS_TYPE_INT32:
    SET_UI64_RESULT(result, value.u32);
    return SYSINFO_RET_OK;
  
  case DBUS_TYPE_DOUBLE:
    SET_DBL_RESULT(result, value.dbl);
    return SYSINFO_RET_OK;
  }

  SET_MSG_RESULT(result, zbx_dsprintf(NULL, "unsupported value type: %c", type));
  return SYSINFO_RET_FAIL;
}
