#ifndef __BEKEN_UTIL_H__
#define __BEKEN_UTIL_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern char * dynamic_string_append(char *old, const char *fmt, ...);
extern void dynamic_string_free(void *ptr);

extern char * dynamic_form_data_append(char *old, const char *key, const char *value);

#endif
