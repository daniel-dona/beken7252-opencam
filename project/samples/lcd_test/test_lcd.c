/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes

 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_lcd.h"
#include "arm_arch.h"

#include "sys_ctrl_pub.h"

#include "drv_model_pub.h"
#include "mem_pub.h"

#include "sys_config.h"
#include "rttlogo.h"
static int test_lcd(int argc, char *argv[])
{
    rt_kprintf("test lcd\r\n");
    lcd_clear(WHITE);
    rt_kprintf("SHOW WHITE\r\n");
    lcd_clear(BLACK);
    rt_kprintf("SHOW BLACK\r\n");
    lcd_clear(BLUE);
    rt_kprintf("SHOW BLUE\r\n");
    lcd_clear(BRED);
    rt_kprintf("SHOW BRED\r\n");
    lcd_clear(GRED);
    rt_kprintf("SHOW GRED\r\n");
    lcd_clear(GBLUE);
    rt_kprintf("SHOW GBLUE\r\n");
    lcd_clear(RED);
    rt_kprintf("SHOW RED\r\n");
    lcd_clear(MAGENTA);
    rt_kprintf("SHOW MAGENTA\r\n");
    lcd_clear(GREEN);
    rt_kprintf("SHOW GREEN\r\n");
    lcd_clear(CYAN);
    rt_kprintf("SHOW CYAN\r\n");
    lcd_clear(YELLOW);
    rt_kprintf("SHOW YELLOW\r\n");
    lcd_clear(BROWN);
    rt_kprintf("SHOW BROWN\r\n");
    lcd_clear(BRRED);
    rt_kprintf("SHOW BRRED\r\n");
    lcd_clear(GRAY);
    rt_kprintf("SHOW GRAY\r\n");
    lcd_clear(GRAY175);
    rt_kprintf("SHOW GRAY175\r\n");
    lcd_clear(GRAY151);
    rt_kprintf("SHOW GRAY151\r\n");
    lcd_clear(GRAY187);
    rt_kprintf("SHOW GRAY187\r\n");
    lcd_clear(GRAY240);
    rt_kprintf("SHOW GRAY240\r\n");
    lcd_clear(WHITE);
     /* 显示 RT-Thread logo */
    lcd_show_image(0, 0, 240, 69, image_rttlogo);
    /* set the background color and foreground color */
    lcd_set_color(WHITE, BLACK);

    /* show some string on lcd */
    lcd_show_string(10, 69, 16, "Hello, RT-Thread!");
    lcd_show_string(10, 69+16, 24, "RT-Thread");
    lcd_show_string(10, 69+16+24, 32, "RT-Thread");

    /* draw a line on lcd */
    lcd_draw_line(0, 69+16+24+32, 240, 69+16+24+32);
    lcd_show_string(10, 69+16+24+32+32, 32, "RT-Thread");


    /* draw a concentric circles */
    //  lcd_draw_point(120, 194);
    //  for (int i = 0; i < 46; i += 4)
    //  {
    //      lcd_draw_circle(120, 194, i);
    //  }

    // /* show some string on lcd */
    //    lcd_show_string(10, 69, 16, "Hello, RT-Thread!");
    //    lcd_show_string(10, 69+16, 24, "RT-Thread");
    //    lcd_show_string(10, 69+16+24, 32, "RT-Thread");
    //    /* draw a line on lcd */
    //    lcd_draw_line(0, 69+16+24+32, 240, 69+16+24+32);

    //       /* draw a concentric circles */
    //       lcd_draw_point(120, 194);
    //       for (int i = 0; i < 46; i += 4)
    //       {
    //           lcd_draw_circle(120, 194, i);
    //       }

      return 0;
}
MSH_CMD_EXPORT(test_lcd,TEST LCD);
