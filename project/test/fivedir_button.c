/*
* 程序清单： 这是一个PIN 设备使用例程
* 例程导出了fivedir_button 命令到控制终端
* 命令调用格式： fivedir_button
* 程序功能： 通过按键按下打印输出对应的按下的方向键。
*/
#include <rtthread.h>
#include <rtdevice.h>
/* 根据硬件电路图 定义各个方向键对应的IO */
#define KEY_UP  (28)
#define KEY_DOWN (19)
#define KEY_LEFT (12)
#define KEY_MID  (13)
#define KEY_RIGHT (25)

#define THREAD_PRIORITY     25
#define THREAD_TIMESLICE    5
#define THREAD_STACK_SIZE   512

static rt_thread_t fivedir_tid = RT_NULL;

static void thread_entry(void *paramer)
{
    /* 按键检测线程中循环检测按键的状态并进行一定的消抖处理 */
    while(1)
    {
        if(rt_pin_read(KEY_UP) == PIN_LOW)
        {
            rt_thread_delay(80);
            if(rt_pin_read(KEY_UP) == PIN_LOW)
            {
                rt_kprintf(" key up is press ...\r\n");
            }
        }

        if(rt_pin_read(KEY_DOWN) == PIN_LOW)
        {
            rt_thread_delay(80);
            if(rt_pin_read(KEY_DOWN) == PIN_LOW)
            {
                rt_kprintf(" key down is press ...\r\n");
            }
        }

         if(rt_pin_read(KEY_LEFT) == PIN_LOW)
        {
            rt_thread_delay(80);
            if(rt_pin_read(KEY_LEFT) == PIN_LOW)
            {
                rt_kprintf(" key left is press ...\r\n");
            }
        }

         if(rt_pin_read(KEY_RIGHT) == PIN_LOW)
        {
            rt_thread_delay(80);
            if(rt_pin_read(KEY_RIGHT) == PIN_LOW)
            {
                rt_kprintf(" key right is press ...\r\n");
            }
        }

        if(rt_pin_read(KEY_MID) == PIN_LOW)
        {
            rt_thread_delay(100);
            if(rt_pin_read(KEY_MID) == PIN_LOW)
            {
                rt_kprintf(" key mid is press ...\r\n");
            }
        }

    }
}

static int fivedir_button(int argc,char *argv[])
{
    /* 初始化五向键的状态，设置为上拉输入 */
    rt_pin_mode(KEY_UP, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_DOWN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_LEFT, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_RIGHT, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_MID, PIN_MODE_INPUT_PULLUP);
    rt_kprintf("KEY INIT...\r\n");

    /* 创建按键检测的线程 */
    fivedir_tid = rt_thread_create("fivedir",
                                    thread_entry, RT_NULL,
                                    THREAD_STACK_SIZE,
                                    THREAD_PRIORITY,THREAD_TIMESLICE);
    if (fivedir_tid != RT_NULL)
        {
            rt_thread_startup(fivedir_tid);
        }
    else
    {
        rt_kprintf("create thread failed...\r\n");
    }


    return 0;
}

/* 导出到msh 命令列表中*/
MSH_CMD_EXPORT(fivedir_button , fivedir_button);
