#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <webnet.h>
#include <module.h>


extern void rt_memory_mem(rt_uint32_t *total,rt_uint32_t *used);

void cgi_mem_handler(struct webnet_session* session)
{
    webnet_session_printf(session, "cgi_mem_handler");
	return;
}
