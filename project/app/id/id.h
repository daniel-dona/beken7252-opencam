#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef char DeviceName[33]; 

typedef struct {
    DeviceName name;
    unsigned int id;
} DeviceInfo;

DeviceInfo *get_device(void);