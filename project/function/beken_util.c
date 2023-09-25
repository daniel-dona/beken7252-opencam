#include <stdarg.h>
#include "rtthread.h"
#include "beken_util.h"

char * dynamic_form_data_append(char *old, const char *key, const char *value)
{
    return dynamic_string_append(old, "%s%s=%s", (old)?"&":"", key, value);
}

char * dynamic_string_append(char *old, const char *fmt, ...)
{
    uint32_t old_size, add_size;
    char *new;
    int n;
    va_list args;

    // get argc for estimate memory size.
    {
        new = (char *)fmt;
        add_size = strlen(fmt) + 2;

        do
        {
             new = strstr(new, "%");

             if(new)
             {
                add_size += 1024;
                new++;
             }
        }while(new);
    }

    old_size = (old) ? strlen(old) : 0;

    new = realloc(old, old_size + add_size);
    if(!new)
    {
        if(old)
            free(old);

        return new;
    }

    va_start(args, fmt);
    n = rt_vsnprintf(new + old_size, add_size - 1, fmt, args);
    va_end(args);

    // force end string.
    if(n > 0)
    {
        old = new + old_size + n;
        *old = '\0';

        new = realloc(new, old_size + n + 4);
    }

    return new;
}

void dynamic_string_free(void *ptr)
{
    if(ptr)
        free(ptr);
}
