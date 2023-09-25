/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for stm32f4xx platform.
 * Created on: 2015-01-16
 */

#include <easyflash.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <rthw.h>
#include <rtthread.h>
#include <flash.h>
#include <flash_pub.h>

/* default ENV set for user */
static const ef_env default_env_set[] = {
        {"user", "user"},
};

static char log_buf[RT_CONSOLEBUF_SIZE];
static struct rt_semaphore env_cache_lock;

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size) {
    EfErrCode result = EF_NO_ERR;

    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);

    rt_sem_init(&env_cache_lock, "env lock", 1, RT_IPC_FLAG_PRIO);

    return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    EF_ASSERT(size % 4 == 0);

    flash_read((unsigned char *)buf, (unsigned long)size, addr);

    return result;
}

/* 
 * size param: The flash size of you want to erase in bytes.
 * return: Returns the size of the actual erase.
 */
static int bk_erase(uint32_t addr, size_t size)
{
    unsigned int _size = size;

    /* Calculate the start address of the flash sector(4kbytes) */
    addr = addr & 0x00FFF000;

    do{
        flash_ctrl(CMD_FLASH_ERASE_SECTOR, &addr);
        addr += 4096;

        if (_size < 4096)
            _size = 0;
        else
            _size -= 4096;

    } while (_size);

    return size; // return true erase size
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;
    EfErrCode sfud_result = EF_NO_ERR;

    /* make sure the start address is a multiple of FLASH_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    bk_erase(addr, size);

    return result;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;
    EfErrCode sfud_result = EF_NO_ERR;

    EF_ASSERT(size % 4 == 0);

    flash_write((unsigned char *)buf, (unsigned long)size, addr);

    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    rt_sem_take(&env_cache_lock, RT_WAITING_FOREVER);
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    rt_sem_release(&env_cache_lock);
}

/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {

#ifdef PRINT_DEBUG

    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    ef_print("[Flash](%s:%ld) ", file, line);
    /* must use vprintf to print */
    rt_vsprintf(log_buf, format, args);
    ef_print("%s", log_buf);
    va_end(args);

#endif

}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    ef_print("[Flash]");
    /* must use vprintf to print */
    rt_vsprintf(log_buf, format, args);
    ef_print("%s", log_buf);
    va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    /* must use vprintf to print */
    rt_vsprintf(log_buf, format, args);
    rt_kprintf("%s", log_buf);
    va_end(args);
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) && defined(EF_USING_ENV)
#include <finsh.h>
#if defined(EF_USING_ENV)
static void __setenv(uint8_t argc, char **argv) {
    uint8_t i;
    char c_value = NULL;
    char *value = &c_value;
    if (argc > 3) {
        /* environment variable value string together */
        for (i = 0; i < argc - 2; i++) {
            argv[2 + i][rt_strlen(argv[2 + i])] = ' ';
        }
    }
    if (argc == 1) {
        ef_set_env(value, value);
    } else if (argc == 2) {
        ef_set_env(argv[1], value);
    } else {
        ef_set_env(argv[1], argv[2]);
    }
}
MSH_CMD_EXPORT_ALIAS(__setenv, setenv, Set an envrionment variable.);

static void printenv(uint8_t argc, char **argv) {
    ef_print_env();
}
MSH_CMD_EXPORT(printenv, Print all envrionment variables.);

static void saveenv(uint8_t argc, char **argv) {
    ef_save_env();
}
MSH_CMD_EXPORT(saveenv, Save all envrionment variables to flash.);

static void getvalue(uint8_t argc, char **argv) {
    char *value = NULL;
    value = ef_get_env(argv[1]);
    if (value) {
        rt_kprintf("The %s value is %s.\n", argv[1], value);
    } else {
        rt_kprintf("Can't find %s.\n", argv[1]);
    }
}
MSH_CMD_EXPORT(getvalue, Get an envrionment variable by name.);

static void resetenv(uint8_t argc, char **argv) {
    ef_env_set_default();
}
MSH_CMD_EXPORT(resetenv, Reset all envrionment variable to default.);
#endif /* defined(EF_USING_ENV) */
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
