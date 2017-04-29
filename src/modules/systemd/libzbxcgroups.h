#include <stdio.h>

// Zabbix source headers
#include <log.h>

int     cgroup_dir_detect();
int     SYSTEMD_CGROUP_CPU(AGENT_REQUEST *request, AGENT_RESULT *result);
//int     SYSTEMD_CGROUP_DEV(AGENT_REQUEST *request, AGENT_RESULT *result);
int     SYSTEMD_CGROUP_MEM(AGENT_REQUEST *request, AGENT_RESULT *result);

