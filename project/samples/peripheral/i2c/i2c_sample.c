/*
 * File      : demo_iic.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-3-15      rt-thread    the first version
 */

#include <string.h>
#include <time.h>
#include <rtthread.h>
#include <rtdevice.h>

#define BCD2BIN(x) (((x)>>4)*10+((x)&0x0f))
#define BIN2BCD(x) ((((x)/10)<<4)|((x)%10))

#define RTC_ADDR        (0x51)

/* RX8564LC register definition */
#define R_Control1          0x00
#define R_Control2          0x01
#define R_Secnods           0x02
#define R_Minutes           0x03
#define R_Hours             0x04
#define R_Days              0x05
#define R_Weekdays          0x06
#define R_Months            0x07
#define R_Years             0x08
#define R_Minute_Alarm      0x09
#define R_Hour_Alarm        0x0A
#define R_Day_Alarm         0x0B
#define R_Weekday_Alarm     0x0C
#define R_CLKOUT_Freq       0x0D
#define R_Timer_Contorl     0x0E
#define R_Timer             0x0F

struct pcf8563_alarm
{
    rt_uint8_t  Control2;           //0x0A
    rt_uint8_t  Minute_Alarm;
    rt_uint8_t  Hour_Alarm;
    rt_uint8_t  Weekday_Alarm;
};

struct i2c_slave
{
    struct rt_device device;
    const char *busname;
};

static rt_size_t rx8564lc_read(rt_device_t dev,
                               rt_uint8_t  offset,
                               rt_uint8_t *data,
                               rt_size_t   size)
{
    struct rt_i2c_msg msgs[2];

    if (dev->user_data == RT_NULL)
        return 0;

    msgs[0].addr  = RTC_ADDR;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf   = &offset;
    msgs[0].len   = 1;         /* Length of msgs.buf */

    msgs[1].addr  = RTC_ADDR;
    msgs[1].flags = RT_I2C_RD; /* Read from slave */
    msgs[1].buf   = data;
    msgs[1].len   = size;

    if (rt_i2c_transfer(dev->user_data, msgs, 2) == 2)
        return size;
    else
        return 0;
}

static rt_size_t rx8564lc_write(rt_device_t dev,
                                rt_uint8_t  offset,
                                rt_uint8_t *data,
                                rt_size_t   size)
{
    struct rt_i2c_msg msgs[2];

    if (dev->user_data == RT_NULL)
        return 0;

    msgs[0].addr  = RTC_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &offset;
    msgs[0].len   = 1;

    msgs[1].addr  = RTC_ADDR;
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;
    msgs[1].buf   = data;
    msgs[1].len   = size;

    if (rt_i2c_transfer(dev->user_data, msgs, 2) == 2)
        return size;
    else
        return 0;
}

static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct i2c_slave *slave;
    rt_uint8_t data[16] = {0};
    rt_err_t ret = RT_EOK;

    RT_ASSERT(dev != RT_NULL);

    /* Assume to be OK. */
    if (dev->user_data)
        return RT_EOK;

    slave = (struct i2c_slave *)dev;

    dev->user_data = rt_device_find(slave->busname);
    if (!dev->user_data)
        return -RT_ERROR;

    if (rx8564lc_read(dev, 0, &data[0], sizeof(data)) != sizeof(data))
    {
        rt_kprintf("RTC read register failed!\n\n");
        goto _out;
    }

    if (data[0] & (1 << 5))
    {
        data[0] &= ~(1 << 5);

        if (rx8564lc_write(dev, 0, &data[0], 1) != 1)
        {
            rt_kprintf("write start RTC failed!\n\n");
            goto _out;
        }

        if (rx8564lc_read(dev, 0, &data[0], 16) != sizeof(data))
        {
            rt_kprintf("RTC read register failed!\n\n");
            goto _out;
        }

        if (data[0] & (1 << 5))
        {
            rt_kprintf("RTC start failed\n");
            goto _out;
        }

        ret = RT_EOK;
    }

_out:
    return ret;
}

static rt_err_t rt_rtc_control(rt_device_t dev, int cmd, void *args)
{
    rt_uint8_t data[16];
    rt_err_t ret = RT_EOK;

    RT_ASSERT(args);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
    {
        struct tm tm = {0};

        if (rx8564lc_read(dev, 0, &data[0], sizeof(data)) != sizeof(data))
        {
            rt_kprintf("read failed\n");
            ret = -RT_ERROR;
            break;
        }

        //rt_kprintf("ALARM : %02X\n", data[1]);
        //rt_kprintf("ALARM : %d  %d:%d\n", BCD2BIN(data[12] & 0x7F), BCD2BIN(data[10] & 0x7F), BCD2BIN(data[9] & 0x7F));

        tm.tm_sec  = BCD2BIN(data[R_Secnods] & 0x7F);      // [6:0] SECONDS (0 to 59)
        tm.tm_min  = BCD2BIN(data[R_Minutes] & 0x7F);      // [6:0] MINUTES (0 to 59)
        tm.tm_hour = BCD2BIN(data[R_Hours] & 0x3F);        // [5:0] HOURS (0 to 23)
        tm.tm_mday = BCD2BIN(data[R_Days] & 0x3F);         // [5:0] DAYS (1 to 31)
        tm.tm_mon  = BCD2BIN(data[R_Months] & 0x1F) - 1;   // [4:0] MONTHS (1 to 12), tm_mon: 0-11
        tm.tm_year = BCD2BIN(data[R_Years]) + 100;         // [7:0] YEARS (0 to 99)
        tm.tm_wday = data[R_Weekdays] & 0x07;              // [2:0] WEEKD AYS (0 to 6)

        *(time_t *)args = mktime(&tm);
        //rt_kprintf("RT_DEVICE_CTRL_RTC_GET_TIME: %d/%d/%d  %d  %d:%d:%d\n", tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_wday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
        struct tm tm = {0};
        tm = *localtime((time_t *)args);

        //rt_kprintf("RT_DEVICE_CTRL_RTC_SET_TIME: %d/%d/%d  %d  %d:%d:%d\n", tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_wday, tm.tm_hour, tm.tm_min, tm.tm_sec);

        data[R_Secnods]  = BIN2BCD(tm.tm_sec);
        data[R_Minutes]  = BIN2BCD(tm.tm_min);
        data[R_Hours]    = BIN2BCD(tm.tm_hour);
        data[R_Days]     = BIN2BCD(tm.tm_mday);
        data[R_Months]   = BIN2BCD((tm.tm_mon + 1));       // data[mon]: 1-12
        data[R_Years]    = BIN2BCD(tm.tm_year - 100);
        data[R_Weekdays] = tm.tm_wday;

        if (rx8564lc_write(dev, R_Secnods, &data[R_Secnods], 7) != 7)
        {
            rt_kprintf("write failed\n");
            ret = -RT_ERROR;
            break;
        }
    }
    break;
    }

    return ret;
}

static struct i2c_slave _rtc;

void rt_hw_pcf8563_init(const char *rtc_name, const char *iicbus_name)
{
    rt_device_t device;
    RT_ASSERT(iicbus_name != RT_NULL);

    _rtc.busname = iicbus_name;

    device = (rt_device_t)&_rtc;

    /* Register RTC device */
    device->type        = RT_Device_Class_RTC;

    device->init        = RT_NULL;
    device->open        = rt_rtc_open;
    device->close       = RT_NULL;
    device->read        = RT_NULL;
    device->write       = RT_NULL;
    device->control     = rt_rtc_control;
    device->user_data   = RT_NULL;

    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;

    rt_device_register(device, rtc_name, RT_DEVICE_FLAG_RDWR);
    rt_kprintf("rt_hw_pcf8563_init\n");
}

static int _cmp_init(void)
{
    rt_hw_pcf8563_init("rtc00", "i2c");
    return 0;
}
INIT_DEVICE_EXPORT(_cmp_init);

struct rt_device *device = RT_NULL;

static rt_uint32_t _atoi(char *src)
{
    rt_uint32_t num = 0;

    RT_ASSERT(NULL != src);

    while ((*src != RT_NULL) && ((*src < '0') || (*src > '9')))
    {
        src++;
    }

    if ((*src == '0') && ((*(src + 1) == 'x') || ((*(src + 1) == 'X'))))
    {
        src += 2;
        while (*src != RT_NULL)
        {
            if ((*src >= '0') && (*src <= '9'))
            {
                num = num * 16 + *src - '0';
            }
            else if ((*src >= 'A') && (*src <= 'F'))
            {
                num = num * 16 + *src - 'A' + 10;
            }
            else if ((*src >= 'a') && (*src <= 'f'))
            {
                num = num * 16 + *src - 'a' + 10;
            }
            else
            {
                return num;
            }
            src++;
        }
    }
    else
    {
        while (*src != RT_NULL)
        {
            if ((*src >= '0') && (*src <= '9'))
            {
                num = num * 10 + *src - '0';
            }
            else
            {
                return num;
            }

            src++;
        }
    }

    return num;
}

rt_err_t pcf8563_set(int argc, char **argv)
{
    rt_uint32_t seconds;

    if (argc != 2)
    {
        rt_kprintf("use:IIC_W 1521163482\n");
        return -RT_ERROR;
    }

    seconds = _atoi(argv[1]);
    if (seconds == 0)
    {
        rt_kprintf("use:IIC_W 1521163482\n");
    }

    rt_kprintf("seconds:%d\n", seconds);

    if (device == RT_NULL)
    {
        device = rt_device_find("rtc00");
        if (device == RT_NULL)
        {
            return -RT_ERROR;
        }
        rt_device_open(device, RT_DEVICE_FLAG_RDWR);
    }

    /* update to RTC device. */
    rt_device_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &seconds);
}

time_t pcf8563_get(int argc, char **argv)
{
    time_t seconds = 0;

    if (device == RT_NULL)
    {
        device = rt_device_find("rtc00");
        if (device == RT_NULL)
        {
            return 0;
        }

        rt_device_open(device, RT_DEVICE_FLAG_RDWR);
    }

    /* update to RTC device. */
    rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &seconds);

    rt_kprintf("seconds:%d\n", seconds);

    return seconds;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT_ALIAS(pcf8563_set, __cmd_IIC_W, IIC write test cmd.);
FINSH_FUNCTION_EXPORT_ALIAS(pcf8563_get, __cmd_IIC_R, IIC READ test cmd.);
#endif


