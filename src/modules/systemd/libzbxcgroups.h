#include <stdio.h>

// Zabbix source headers
#define HAVE_TIME_H 1
#include <sysinc.h>
#include <module.h>
#include <common.h>
#include <log.h>
#include <zbxjson.h>
#include <version.h>
#include <db.h>

// cgroup mount directory
char *cgroup_dir = NULL;

int     cgroup_dir_detect();
