#include "libzbxsystemd.h"
#include "libzbxcgroups.h"

/******************************************************************************
 *                                                                            *
 * Function: cgroup_dir_detect                                                *
 *                                                                            *
 * Purpose: it should find cgroup metric directory                            *
 *                                                                            *
 * Return value: SYSINFO_RET_FAIL - cgroup directory was not found            *
 *               SYSINFO_RET_OK - cgroup directory was found                  *
 *                                                                            *
 ******************************************************************************/
int     cgroup_dir_detect()
{
        zabbix_log(LOG_LEVEL_DEBUG, LOG_PREFIX "in cgroup_dir_detect()");

        char path[512];
        char *temp1, *temp2;
        FILE *fp;

        if ((fp = fopen("/proc/mounts", "r")) == NULL)
        {
            zabbix_log(LOG_LEVEL_WARNING, LOG_PREFIX "cannot open /proc/mounts: %s", zbx_strerror(errno));
            return SYSINFO_RET_FAIL;
        }

        while (fgets(path, 512, fp) != NULL)
        {
            if ((strstr(path, "cpuset cgroup")) != NULL)
            {
                temp1 = string_replace(path, "cgroup ", "");
                temp2 = string_replace(temp1, strstr(temp1, " "), "");
                free(temp1);
                cgroup_dir = string_replace(temp2, "cpuset", "");
                free(temp2);
                zabbix_log(LOG_LEVEL_DEBUG, LOG_PREFIX "detected cgroup mount directory: %s", cgroup_dir);
                pclose(fp);
                return SYSINFO_RET_OK;
            }
        }
        pclose(fp);
        zabbix_log(LOG_LEVEL_DEBUG, LOG_PREFIX "cannot detect cgroup mount directory");
        return SYSINFO_RET_FAIL;
}

