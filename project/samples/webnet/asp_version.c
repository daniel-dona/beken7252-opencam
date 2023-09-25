#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <webnet.h>
#include <module.h>

void asp_var_version(struct webnet_session* session)
{
    webnet_session_printf(session,
                          "RT-Thread %d.%d.%d",
                          RT_VERSION,
                          RT_SUBVERSION,
                          RT_REVISION);
}
