#include <string.h>

#include <rtthread.h>
#include "include.h"

#include <easyflash.h>
#include <cJSON_util.h>
#include <finsh.h>
#include <dfs_posix.h>



#include "user_config.h"

/*
char *ef_get_env(const char *key);
EfErrCode ef_set_env(const char *key, const char *value);
*/

/*
uint8_t read_user_config_sd(void){

    int fd;
    cJSON *json = RT_NULL;

    fd = open(filename, O_RDONLY, 0);




}

*/

int parse_config_file(const char *filename, cJSON **json_result){

    int fd;
    int result = -1;
    *json_result = RT_NULL;

    fd = open(filename, O_RDONLY, 0);
    if (fd < 0)
    {
        /* no setting file */
        rt_kprintf("Config file not found: %s\r\n", filename);
        return -1;
    }

    if (fd >= 0)
    {
        int length;

        length = lseek(fd, 0, SEEK_END);
        if (length > 0)
        {
            char *json_str = (char *) rt_malloc(length + 1);
            if (json_str)
            {
                lseek(fd, 0, SEEK_SET);
                read(fd, json_str, length);
                json_str[length] = '\0'; // Null-terminate the string
                
                //rt_kprintf("JSON content from %s: %s\r\n", filename, json_str);

                *json_result = cJSON_Parse(json_str);
                if (*json_result == RT_NULL)
                {
                    rt_kprintf("Failed to parse JSON from: %s\r\n", filename);
                    result = -2; // Parse error
                }
                else
                {
                    rt_kprintf("Successfully parsed: %s\r\n", filename);
                    result = 0; // Success
                }
                
                rt_free(json_str);
            }
            else
            {
                rt_kprintf("Memory allocation failed for: %s\r\n", filename);
                result = -3; // Memory error
            }
        }
        else
        {
            rt_kprintf("Empty or invalid file: %s\r\n", filename);
            result = -4; // Empty file
        }
        
        close(fd);
    }

    return result;
}

void print_all_configs() {
    bk_printf("Configuration Mappings:\n");
    bk_printf("=======================\n");
    
    for (int i = 0; i < CONFIG_MAPPINGS_COUNT; i++) {
        bk_printf("%2d. File: %-40s Key: %s\n", 
               i, 
               config_mappings[i].file_path, 
               config_mappings[i].ef_key);
    }
}

const config_mapping_t* find_config_by_key(const char* key) {
    for (int i = 0; i < CONFIG_MAPPINGS_COUNT; i++) {
        if (strcmp(config_mappings[i].ef_key, key) == 0) {
            return &config_mappings[i];
        }
    }
    return NULL;
}



MSH_CMD_EXPORT(print_all_configs, Print the defined configuration files and keys);
