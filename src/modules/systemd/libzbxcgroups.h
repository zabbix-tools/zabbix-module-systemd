#include <stdio.h>

// Zabbix source headers
#include <log.h>

int     cgroup_dir_detect();
int     SYSTEMD_CGROUP_CPU(AGENT_REQUEST*, AGENT_RESULT*);
//int     SYSTEMD_CGROUP_DEV(AGENT_REQUEST*, AGENT_RESULT*);
int     SYSTEMD_CGROUP_MEM(AGENT_REQUEST*, AGENT_RESULT*);

