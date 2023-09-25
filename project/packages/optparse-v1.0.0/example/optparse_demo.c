#include "optparse.h"
#include "finsh.h"

int optparse_short_test(int argc, char **argv)
{
    int ch; 
    struct optparse options;
    
    optparse_init(&options, argv); 
    while((ch = optparse(&options, "ab:c::")) != -1)
    {
        ch = ch; 
        
        rt_kprintf("\n"); 
        rt_kprintf("optopt = %c\n", options.optopt);
        rt_kprintf("optarg = %s\n", options.optarg);
        rt_kprintf("optind = %d\n", options.optind);
    }
    rt_kprintf("\n");
    
    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(optparse_short_test, ost, test optparse_short cmd.); 

static struct optparse_long long_opts[] = 
{
    {"aaa", 'a', OPTPARSE_NONE    }, 
    {"bbb", 'b', OPTPARSE_REQUIRED}, 
    {"ccc", 'c', OPTPARSE_OPTIONAL}, 
    { NULL,  0,  OPTPARSE_NONE    }
}; 

int optparse_long_test(int argc, char **argv)
{
    int ch; 
    int option_index; 
    struct optparse options;
    
    optparse_init(&options, argv); 
    while((ch = optparse_long(&options, long_opts, &option_index)) != -1)
    {
        ch = ch; 
        
        rt_kprintf("\n"); 
        rt_kprintf("optopt = %c\n", options.optopt);
        rt_kprintf("optarg = %s\n", options.optarg);
        rt_kprintf("optind = %d\n", options.optind);
        rt_kprintf("option_index = %d\n", option_index); 
    }
    rt_kprintf("\n");
    
    return RT_EOK; 
}
MSH_CMD_EXPORT_ALIAS(optparse_long_test, olt, test optparse_long cmd.); 
