#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <dfs_posix.h>
#include <sys/select.h>
#include "lssdp.h"
#include "lssdp_service.h"

/* daemon.c
 *
 * 1. create SSDP socket with port 1900
 * 2. select SSDP socket with timeout 0.5 seconds
 *    - when select return value > 0, invoke lssdp_socket_read
 * 3. per 5 seconds do:
 *    - update network interface
 *    - send M-SEARCH and NOTIFY
 *    - check neighbor timeout
 * 4. when neighbor list is changed
 *    - show neighbor list
 * 5. when network interface is changed
 *    - show interface list
 *    - re-bind the socket
 */
 
#define DBG_SECTION_NAME  "lssdp service"
#define DBG_LEVEL         LOG_LVL_WARNING
#include <rtdbg.h>

void log_callback(const char * file, const char * tag, int level, int line, const char * func, const char * message) {
    char * level_name = "DEBUG";
    if (level == LSSDP_LOG_INFO)   level_name = "INFO";
    if (level == LSSDP_LOG_WARN)   level_name = "WARN";
    if (level == LSSDP_LOG_ERROR)  level_name = "ERROR";

    rt_kprintf("[%-5s][%s] %s", level_name, tag, message);
}

long long get_current_time() {
    struct timeval time;
    if (gettimeofday(&time, NULL) == -1) {
        LOG_W("gettimeofday failed, errno = %s (%d)\n", strerror(errno), errno);
        return -1;
    }
    return (long long) time.tv_sec * 1000 + (long long) time.tv_usec / 1000;
}

int show_neighbor_list(lssdp_ctx * lssdp) {
    int i = 0;
    lssdp_nbr * nbr;
    LOG_I("\nSSDP List:\n");
    for (nbr = lssdp->neighbor_list; nbr != NULL; nbr = nbr->next) {
        rt_kprintf("%d. id = %-9s, ip = %-20s, name = %-12s, device_type = %-8s (%lld)\n",
               ++i,
               nbr->sm_id,
               nbr->location,
               nbr->usn,
               nbr->device_type,
               nbr->update_time
              );
    }
    rt_kprintf("%s\n", i == 0 ? "Empty" : "");
    return 0;
}

int show_interface_list_and_rebind_socket(lssdp_ctx * lssdp) {
    // 1. show interface list
    LOG_I("LSSDP network interface list (%u):", lssdp->interface_num);
    size_t i;
    for (i = 0; i < lssdp->interface_num; i++) {
        LOG_I("%u. %-6s: %s\n",
               i + 1,
               lssdp->interface[i].name,
               lssdp->interface[i].ip
              );
    }
    LOG_I("%s\n", i == 0 ? "Empty" : "");

    // 2. re-bind SSDP socket
    if (lssdp_socket_create(lssdp) != 0) {
        LOG_E("SSDP create socket failed");
        return -1;
    }

    return 0;
}

int lssdp_daemon(void) {
    lssdp_set_log_callback(log_callback);

    lssdp_ctx lssdp = {
        .debug = false,           // debug
        .port = 1900,
        .neighbor_timeout = 15000,  // 15 seconds

        .header.location.prefix = "http://",

        // callback
        .neighbor_list_changed_callback     = show_neighbor_list,
        .network_interface_changed_callback = show_interface_list_and_rebind_socket,
    };

    /* get network interface at first time, network_interface_changed_callback will be invoke
     * SSDP socket will be created in callback function
     */
    lssdp_network_interface_update(&lssdp);

    long long last_time = get_current_time();
    if (last_time < 0) {
        LOG_W("got invalid timestamp %lld\n", last_time);
        return EXIT_SUCCESS;
    }

    for (;;) {
        fd_set fs;
        FD_ZERO(&fs);
        FD_SET(lssdp.sock, &fs);
        struct timeval tv = {
            .tv_usec = 500 * 1000   // 500 ms
        };

        int ret = select(lssdp.sock + 1, &fs, NULL, NULL, &tv);
        if (ret < 0) {
            LOG_W("select error, ret = %d\n", ret);
            break;
        }

        if (ret > 0) {
            lssdp_socket_read(&lssdp);
        }

        // get current time
        long long current_time = get_current_time();
        if (current_time < 0) {
            LOG_W("got invalid timestamp %lld\n", current_time);
            break;
        }

        // doing task per 5 seconds
        if (current_time - last_time >= 2000) {
            lssdp_network_interface_update(&lssdp); // update network interface
            lssdp_service_send_notify(&lssdp);      // send notify messages to network

//            lssdp_send_msearch(&lssdp);             // 2. send M-SEARCH
//            lssdp_send_notify(&lssdp);              // 3. send NOTIFY
//            lssdp_neighbor_check_timeout(&lssdp);   // 4. check neighbor timeout

            last_time = current_time;               // update last_time
        }
    }

    return EXIT_SUCCESS;
}

