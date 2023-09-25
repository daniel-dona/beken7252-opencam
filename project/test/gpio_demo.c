/*
* 程序清单： 这是一个PIN 设备使用例程
* 例程导出了pin_led_sample 命令到控制终端
* 命令调用格式： pin_led_sample
* 程序功能： 通过按键控制led 对应引脚的电平状态控制led
*/
#include <rtthread.h>
#include <rtdevice.h>

#define LED_PIN_NUM 30
#define LED1_PIN_NUM 13
#define LED2_PIN_NUM 27
#define KEY0_PIN_NUM 2
#define KEY1_PIN_NUM 3

void led_on(void *args) 
{
	rt_kprintf("turn on led!\n");
	rt_pin_write(LED_PIN_NUM, PIN_HIGH);
}

void led_off(void *args) 
{
	rt_kprintf("turn off led!\n");
	rt_pin_write(LED_PIN_NUM, PIN_LOW);
}

static void pin_led_sample(void) 
{
	/* led 引脚为输出模式*/
	rt_pin_mode(LED_PIN_NUM, PIN_MODE_OUTPUT);
	/* 默认低电平*/
	rt_pin_write(LED_PIN_NUM, PIN_LOW);
	/* 按键0引脚为输入模式*/
	rt_pin_mode(KEY0_PIN_NUM , PIN_MODE_INPUT_PULLUP);
	/* 绑定中断， 下降沿模式， 回调函数名为beep_on */
	rt_pin_attach_irq(KEY0_PIN_NUM , PIN_IRQ_MODE_FALLING , led_on, RT_NULL);
	/* 使能中断*/
	rt_pin_irq_enable(KEY0_PIN_NUM , PIN_IRQ_ENABLE);
	/* 按键1引脚为输入模式*/
	rt_pin_mode(KEY1_PIN_NUM , PIN_MODE_INPUT_PULLUP);
	/* 绑定中断， 下降沿模式， 回调函数名为led_off */
	rt_pin_attach_irq(KEY1_PIN_NUM , PIN_IRQ_MODE_FALLING , led_off, RT_NULL);
	/* 使能中断*/
	rt_pin_irq_enable(KEY1_PIN_NUM , PIN_IRQ_ENABLE);

	/* led 引脚为输出模式*/
	rt_pin_mode(LED1_PIN_NUM, PIN_MODE_OUTPUT);
	/* 默认低电平*/
	rt_pin_write(LED1_PIN_NUM, PIN_LOW);
	/* led 引脚为输出模式*/
	rt_pin_mode(LED2_PIN_NUM, PIN_MODE_OUTPUT);
	/* 默认低电平*/
	rt_pin_write(LED2_PIN_NUM, PIN_HIGH);
}
/* 导出到msh 命令列表中*/
MSH_CMD_EXPORT(pin_led_sample , pin led sample);
