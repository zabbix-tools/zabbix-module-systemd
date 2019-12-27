#include "libzbxsystemd.h"

// pid that initialised the module, before forking workers.
int mainpid = 0;

// items in this file
static int SYSTEMD_MODVER(AGENT_REQUEST*, AGENT_RESULT*);
static int SYSTEMD_MANAGER(AGENT_REQUEST *request, AGENT_RESULT *result);
static int SYSTEMD_UNIT(AGENT_REQUEST*, AGENT_RESULT*);
static int SYSTEMD_UNIT_DISCOVERY(AGENT_REQUEST*, AGENT_RESULT*);
static int SYSTEMD_SERVICE_INFO(AGENT_REQUEST*, AGENT_RESULT*);
static int SYSTEMD_SERVICE_DISCOVERY(AGENT_REQUEST*, AGENT_RESULT*);

// items in cgroups.c
int cgroup_init();
int SYSTEMD_CGROUP_CPU(AGENT_REQUEST*, AGENT_RESULT*);
int SYSTEMD_CGROUP_DEV(AGENT_REQUEST*, AGENT_RESULT*);
int SYSTEMD_CGROUP_MEM(AGENT_REQUEST*, AGENT_RESULT*);

ZBX_METRIC *zbx_module_item_list()
{
  static ZBX_METRIC keys[] =
  {
    { "systemd.modver",             0,              SYSTEMD_MODVER,             NULL },
    { "systemd",                    CF_HAVEPARAMS,  SYSTEMD_MANAGER,            "Version" },
    { "systemd.unit",               CF_HAVEPARAMS,  SYSTEMD_UNIT,               "dbus.service,Service,Result" },
    { "systemd.unit.discovery",     CF_HAVEPARAMS,  SYSTEMD_UNIT_DISCOVERY,     NULL },
    { "systemd.service.info",       CF_HAVEPARAMS,  SYSTEMD_SERVICE_INFO,       "dbus.service" },
    { "systemd.service.discovery",  CF_HAVEPARAMS,  SYSTEMD_SERVICE_DISCOVERY,  NULL },
    { "systemd.cgroup.cpu",         CF_HAVEPARAMS,  SYSTEMD_CGROUP_CPU,         "dbus.service,total" },
    { "systemd.cgroup.dev",         CF_HAVEPARAMS,  SYSTEMD_CGROUP_DEV,         "dbus.service,blkio.io_queued,Total" },
    { "systemd.cgroup.mem",         CF_HAVEPARAMS,  SYSTEMD_CGROUP_MEM,         "dbus.service,rss" },
    { NULL }
  };

  return keys;
}

int zbx_module_api_version() {
  return ZBX_MODULE_API_VERSION;
}

int zbx_module_init()
{
    zabbix_log(LOG_LEVEL_DEBUG, LOG_PREFIX "starting v%s, compiled: %s %s", PACKAGE_VERSION, __DATE__, __TIME__);
    mainpid = getpid();
    cgroup_init();
    return ZBX_MODULE_OK;
}

int zbx_module_uninit()
{
  if (NULL != conn)
    dbus_connection_unref(conn);
  return ZBX_MODULE_OK;
}

int timeout = 3000;
void zbx_module_item_timeout(int t)
{
    timeout = t * 1000;
    zabbix_log(LOG_LEVEL_DEBUG, LOG_PREFIX "set timeout to %ims", timeout);
}

static int SYSTEMD_MODVER(AGENT_REQUEST *request, AGENT_RESULT *result)
{
    SET_STR_RESULT(result, strdup(PACKAGE_STRING ", compiled with Zabbix " ZABBIX_VERSION));
    return SYSINFO_RET_OK;
}

// systemd[<property=Version>]
static int SYSTEMD_MANAGER(AGENT_REQUEST *request, AGENT_RESULT *result)
{
  const char      *property;

  if (1 < request->nparam) {
    SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
    return SYSINFO_RET_FAIL;
  }
  
  // resolve property name
  property = get_rparam(request, 0);
  if (NULL == property || '\0' == *property)
    property = "Version";

  if (FAIL == dbus_connect()) {
    SET_MSG_RESULT(result, strdup("Failed to connect to D-Bus."));
    return SYSINFO_RET_FAIL;
  }

  // get value
  return dbus_marshall_property(
    result,
    SYSTEMD_SERVICE_NAME,
    SYSTEMD_ROOT_NODE,
    SYSTEMD_MANAGER_INTERFACE,
    property
  );
}

// systemd.unit[unit_name,<interface=Unit>,<property=ActiveState>]
static int SYSTEMD_UNIT(AGENT_REQUEST *request, AGENT_RESULT *result)
{
  const char      *unit, *interface, *property;
  char            path[4096], buf[DBUS_MAXIMUM_NAME_LENGTH+1];
  int             res = SYSINFO_RET_FAIL;

  if (1 > request->nparam || 3 < request->nparam) {
    SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
    return SYSINFO_RET_FAIL;
  }
  
  if (FAIL == dbus_connect()) {
    SET_MSG_RESULT(result, strdup("Failed to connect to D-Bus."));
    return SYSINFO_RET_FAIL;
  }
  
  // resolve unit name to object path
  unit = get_rparam(request, 0);
  if (FAIL == systemd_get_unit(path, sizeof(path), unit)) {
    SET_MSG_RESULT(result, strdup("unit not found"));
    return res;
  }

  // resolve full interface name (default: org.freedesktop.systemd1.Unit)
  interface = get_rparam(request, 1);
  if (NULL == interface || '\0' == *interface) {
    interface = SYSTEMD_SERVICE_NAME ".Unit";
  } else {
    zbx_snprintf(buf, sizeof(buf), SYSTEMD_SERVICE_NAME ".%s", interface);
    interface = &buf[0];
  }

  // resolve property name (default: ActiveState)
  property = get_rparam(request, 2);
  if (NULL == property || '\0' == *property)
    property = "ActiveState";

  // get value
  return dbus_marshall_property(
    result,
    SYSTEMD_SERVICE_NAME,
    path,
    interface,
    property
  );
}

// systemd.unit.discovery[]
static int SYSTEMD_UNIT_DISCOVERY(AGENT_REQUEST *request, AGENT_RESULT *result)
{
  DBusMessage     *msg = NULL;
  DBusMessageIter args, arr, unit;
  DBusBasicValue  value;
  struct zbx_json j;
  const char      *filter;
  int             res = SYSINFO_RET_FAIL;
  int             type = 0, i = 0;

  if (1 < request->nparam) {
    SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
    return SYSINFO_RET_FAIL;
  }

  filter = get_rparam(request, 0);

  if (FAIL == dbus_connect()) {
    SET_MSG_RESULT(result, strdup("Failed to connect to D-Bus."));
    return SYSINFO_RET_FAIL;
  }

  // send method call
  msg = dbus_message_new_method_call(
    SYSTEMD_SERVICE_NAME,
    SYSTEMD_ROOT_NODE,
    SYSTEMD_MANAGER_INTERFACE,
    "ListUnits");

  if (NULL == (msg = dbus_exchange_message(msg))) {
    SET_MSG_RESULT(result, strdup("failed to list units"));
    dbus_message_unref(msg);
    return res;
  }

  // check result message
  if (!dbus_message_iter_init(msg, &args)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "no value returned");
    dbus_message_unref(msg);
    return res;
  }
  
  if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "returned value is not an array");
    dbus_message_unref(msg);
    return res;
  }
  
  zbx_json_init(&j, ZBX_JSON_STAT_BUF_LEN);
  zbx_json_addarray(&j, ZBX_PROTO_TAG_DATA);

  // loop through returned units
  dbus_message_iter_recurse(&args, &arr);
  while ((type = dbus_message_iter_get_arg_type (&arr)) != DBUS_TYPE_INVALID) {
    dbus_message_iter_recurse(&arr, &unit);

    // loop through values
    i = 0;
    while((type = dbus_message_iter_get_arg_type(&unit)) != DBUS_TYPE_INVALID) {
      dbus_message_iter_get_basic(&unit, &value);

      switch (i) {
      case 0:
        // filter by unit type
        if(NULL != filter || '\0' != filter)
          if(0 == systemd_cmptype(value.str, filter))
            goto next_unit;

        zbx_json_addobject(&j, NULL);
        zbx_json_addstring(&j, "{#UNIT.NAME}", value.str, ZBX_JSON_TYPE_STRING);
        break;

      case 1:
        zbx_json_addstring(&j, "{#UNIT.DESCRIPTION}", value.str, ZBX_JSON_TYPE_STRING);
        break;

      case 2:
        zbx_json_addstring(&j, "{#UNIT.LOADSTATE}", value.str, ZBX_JSON_TYPE_STRING);
        break;

      case 3:
        zbx_json_addstring(&j, "{#UNIT.ACTIVESTATE}", value.str, ZBX_JSON_TYPE_STRING);
        break;
      
      case 4:
        zbx_json_addstring(&j, "{#UNIT.SUBSTATE}", value.str, ZBX_JSON_TYPE_STRING);
        break;
      
      case 6:
        zbx_json_addstring(&j, "{#UNIT.OBJECTPATH}", value.str, ZBX_JSON_TYPE_STRING);
        
        // while we know the object path, lookup additional properties
        dbus_get_property_json(&j, "{#UNIT.FRAGMENTPATH}", value.str, SYSTEMD_UNIT_INTERFACE, "FragmentPath");
        dbus_get_property_json(&j, "{#UNIT.UNITFILESTATE}", value.str, SYSTEMD_UNIT_INTERFACE, "UnitFileState");
        dbus_get_property_json(&j, "{#UNIT.FOLLOWING}", value.str, SYSTEMD_UNIT_INTERFACE, "Following");
        dbus_get_property_json(&j, "{#UNIT.CONDITIONRESULT}", value.str, SYSTEMD_UNIT_INTERFACE, "ConditionResult");
        zbx_json_close(&j);
        break;
      }

      dbus_message_iter_next (&unit);
      i++;
    }

  next_unit:
    dbus_message_iter_next(&arr);
  }

  dbus_message_unref(msg);
  zbx_json_close(&j);
  SET_STR_RESULT(result, strdup(j.buffer));
  zbx_json_free(&j);

  return SYSINFO_RET_OK;
}

// service.info[service,<param=state>]
// https://support.zabbix.com/browse/ZBXNEXT-2871
static int SYSTEMD_SERVICE_INFO(AGENT_REQUEST *request, AGENT_RESULT *result)
{
  int         status, paramId = 0;
  char        path[4096], buf[64];
  const char  *service, *param;
  const char  *params[] = {
    "state", "displayname", "path", "user", "startup", "description",
    NULL
  };

  if (1 > request->nparam || 2 < request->nparam) {
    SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
    return SYSINFO_RET_FAIL;
  }

  service = get_rparam(request, 0);
  if (NULL == service || '\0' == *service) {
    SET_MSG_RESULT(result, strdup("Invalid service name."));
    return SYSINFO_RET_FAIL;
  }

  // validate param
  param = get_rparam(request, 1);
  if (NULL != param && '\0' != *param)
    for (paramId = 0; params[paramId]; paramId++)
      if (0 == strncmp(param, params[paramId], 12))
        break;

  if (NULL == params[paramId]) {
    SET_MSG_RESULT(result, zbx_dsprintf(NULL, "Unsupported param: %s", param));
    return SYSINFO_RET_FAIL;
  }

  if (FAIL == dbus_connect()) {
    SET_MSG_RESULT(result, strdup("Failed to connect to D-Bus."));
    return SYSINFO_RET_FAIL;
  }

  // get service object path
  if (FAIL == (systemd_get_unit(path, sizeof(path), service))) {
    SET_MSG_RESULT(result, strdup("Failed to lookup object path"));
    return SYSINFO_RET_FAIL;
  }

  if (!systemd_unit_is_service(path)) {
    SET_MSG_RESULT(result, strdup("Not a service"));
    return SYSINFO_RET_FAIL;
  }

  switch(paramId) {
    case 0: // param = state
      if(FAIL == dbus_get_property_string(
                            buf,
                            sizeof(buf),
                            SYSTEMD_SERVICE_NAME,
                            path,
                            SYSTEMD_UNIT_INTERFACE,
                            "ActiveState")
      ) {
        SET_MSG_RESULT(result, strdup("Failed to get ActiveState property"));
        return SYSINFO_RET_FAIL;
      }                            

      if(-1 == (status = systemd_service_state_code(buf))) {
        SET_MSG_RESULT(result, zbx_dsprintf(NULL, "Unknown service state: %s", buf));
        return SYSINFO_RET_FAIL;
      }

      SET_UI64_RESULT(result, status);
      return SYSINFO_RET_OK;

    case 1: // param = displayname
      return dbus_marshall_property(
                            result,
                            SYSTEMD_SERVICE_NAME,
                            path,
                            SYSTEMD_UNIT_INTERFACE,
                            "Id");

    case 2: // param = path
      // extract path from Service.ExecStart
      if (FAIL == systemd_get_service_path(buf, sizeof(buf), path)) {
        SET_MSG_RESULT(result, strdup("Failed to get ExecStart property"));
        return SYSINFO_RET_FAIL;
      }

      SET_STR_RESULT(result, strdup(buf));
      return SYSINFO_RET_OK;

    case 3: // param = user
      // TODO: Service.User always seems to be empty - maybe use PID?
      return dbus_marshall_property(
                            result,
                            SYSTEMD_SERVICE_NAME,
                            path,
                            SYSTEMD_SERVICE_INTERFACE,
                            "User");

    case 4: // param = startup
      if(FAIL == dbus_get_property_string(
                            buf,
                            sizeof(buf),
                            SYSTEMD_SERVICE_NAME,
                            path,
                            SYSTEMD_UNIT_INTERFACE,
                            "UnitFileState")
      ) {
        SET_MSG_RESULT(result, strdup("Failed to get UnitFileState property"));
        return SYSINFO_RET_FAIL;
      }                            

      if(-1 == (status = systemd_service_startup_code(buf))) {
        SET_MSG_RESULT(result, zbx_dsprintf(NULL, "Unknown service startup: %s", buf));
        return SYSINFO_RET_FAIL;
      }

      SET_UI64_RESULT(result, status);
      return SYSINFO_RET_OK;

    case 5: // param = description
          return dbus_marshall_property(
                            result,
                            SYSTEMD_SERVICE_NAME,
                            path,
                            SYSTEMD_UNIT_INTERFACE,
                            "Description");
  }

  // impossible
  zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "bug in SYSTEMD_SERVICE_INFO(%s, %s)", service, param);
  return SYSINFO_RET_FAIL;
}

// systemd.service.discovery[]
static int SYSTEMD_SERVICE_DISCOVERY(AGENT_REQUEST *request, AGENT_RESULT *result)
{
  DBusMessage     *msg = NULL;
  DBusMessageIter args, arr, unit;
  struct zbx_json j; 
  DBusBasicValue  value;
  int             res = SYSINFO_RET_FAIL;
  int             type = 0;
  char            *path;

  if (FAIL == dbus_connect()) {
    SET_MSG_RESULT(result, strdup("Failed to connect to D-Bus."));
    return SYSINFO_RET_FAIL;
  }

  // send method call
  msg = dbus_message_new_method_call(
    SYSTEMD_SERVICE_NAME,
    SYSTEMD_ROOT_NODE,
    SYSTEMD_MANAGER_INTERFACE,
    "ListUnits");

  if (NULL == (msg = dbus_exchange_message(msg))) {
    SET_MSG_RESULT(result, strdup("failed to list units"));
    dbus_message_unref(msg);
    return res;
  }

  // check result message
  if (!dbus_message_iter_init(msg, &args)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "no value returned");
    dbus_message_unref(msg);
    return res;
  }
  
  if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
    zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "returned value is not an array");
    dbus_message_unref(msg);
    return res;
  }
  
  zbx_json_init(&j, ZBX_JSON_STAT_BUF_LEN);
  zbx_json_addarray(&j, ZBX_PROTO_TAG_DATA);

  // loop through returned units
  dbus_message_iter_recurse(&args, &arr);
  while ((type = dbus_message_iter_get_arg_type (&arr)) != DBUS_TYPE_INVALID) {
    dbus_message_iter_recurse(&arr, &unit);

    // get object path a(ssssssouso)[n][6]
    dbus_message_iter_next_n(&unit, 6);
    if (DBUS_TYPE_INVALID == (type = dbus_message_iter_get_arg_type(&unit))) {
      zabbix_log(LOG_LEVEL_ERR, LOG_PREFIX "unexpected value type");
      dbus_message_unref(msg);
      goto next_unit;
    }

    dbus_message_iter_get_basic(&unit, &value);
    if (!systemd_unit_is_service(value.str))
      goto next_unit;

    // send property values
    zbx_json_addobject(&j, NULL);
    zbx_json_addstring(&j, "{#SERVICE.TYPE}", "service", ZBX_JSON_TYPE_STRING);
    dbus_get_property_json(&j, "{#SERVICE.NAME}", value.str, SYSTEMD_UNIT_INTERFACE, "Id");
    dbus_get_property_json(&j, "{#SERVICE.DISPLAYNAME}", value.str, SYSTEMD_UNIT_INTERFACE, "Description");
    dbus_get_property_json(&j, "{#SERVICE.PATH}", value.str, SYSTEMD_UNIT_INTERFACE, "FragmentPath");
    dbus_get_property_json(&j, "{#SERVICE.STARTUPNAME}", value.str, SYSTEMD_UNIT_INTERFACE, "UnitFileState");
    dbus_get_property_json(&j, "{#SERVICE.CONDITIONRESULT}", value.str, SYSTEMD_UNIT_INTERFACE, "ConditionResult");
    zbx_json_close(&j);

next_unit:
    dbus_message_iter_next(&arr);
  }

  dbus_message_unref(msg);
  zbx_json_close(&j);
  SET_STR_RESULT(result, strdup(j.buffer));
  zbx_json_free(&j);

  return SYSINFO_RET_OK;
}
