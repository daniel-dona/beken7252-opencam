#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "[IIC1]"
#define DBG_LEVEL DBG_INFO
// #define DBG_LEVEL         DBG_INFO
// #define DBG_LEVEL         DBG_WARNING
// #define DBG_LEVEL         DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#define I2C_CLK (20)
#define I2C_DAT (21)

#if defined(RT_USING_I2C) && defined(RT_USING_I2C1)

#include "typedef.h"
#include "drv_iic.h"
#include "gpio.h"
#include "gpio_pub.h"
#include "arm_arch.h"

#include <i2c1.h>
#include <i2c2.h>
#include <i2c_pub.h>
#include "intc_pub.h"
#include "icu_pub.h"
#include "gpio_pub.h"

typedef struct i2c1_msg
{
    UINT8 TxMode; //0: Read;  1: Write
    UINT8 RegAddr;
    UINT16 RemainNum;
    UINT8 *pData;
    volatile UINT8 TransDone;
} I2C1_MSG_ST, *I2C1_MSG_PTR;

static struct rt_completion ack;

static struct rt_i2c_bus_device i2c_bus1;
static I2C1_MSG_ST gi2c1;

static void i2c1_isr(void)
{
    UINT32 I2cStat;

    I2cStat = REG_READ(REG_I2C1_CONFIG);
    if (I2cStat & I2C1_SI)
    {
        if (I2cStat & I2C1_STA)
        {
            I2cStat &= ~I2C1_STA; //Clear STA
            REG_WRITE(REG_I2C1_CONFIG, I2cStat);

            if (!(I2cStat & I2C1_ACK_RX))
            {
                I2cStat |= I2C1_STO;
                REG_WRITE(REG_I2C1_CONFIG, I2cStat); //Set STO
                // gi2c1.TransDone = 1; // addr+NAK
                rt_completion_done(&ack);
                //dbg_log(DBG_LOG, "%s:%d\n", __FUNCTION__, __LINE__);
            }
            else
            {
                if (gi2c1.TxMode)
                {
                    // REG_WRITE(REG_I2C1_DAT, gi2c1.RegAddr); //Tx Reg Address
                    REG_WRITE(REG_I2C1_DAT, *(gi2c1.pData++));
                    gi2c1.RemainNum--;
                }
                else
                {
                    I2cStat &= ~I2C1_TX_MODE;
                    REG_WRITE(REG_I2C1_CONFIG, I2cStat); //Clear TXMODE
                }
            }
        }
        else
        {
            if (gi2c1.TxMode)
            {
                //Reg Write
                if (!(I2cStat & I2C1_ACK_RX) || (gi2c1.RemainNum == 0))
                {
                    //Rx NACK or All Data TX Done
                    I2cStat |= I2C1_STO;
                    REG_WRITE(REG_I2C1_CONFIG, I2cStat); //Set STO
                    // gi2c1.TransDone = 1; // NAK or done.
                    rt_completion_done(&ack);
                    //dbg_log(DBG_LOG, "%s:%d\n", __FUNCTION__, __LINE__);
                }
                else
                {
                    REG_WRITE(REG_I2C1_DAT, *(gi2c1.pData++));
                    gi2c1.RemainNum--;
                }
            }
            else
            {
                //Reg Read
                *(gi2c1.pData++) = REG_READ(REG_I2C1_DAT);
                gi2c1.RemainNum--;
                if (gi2c1.RemainNum)
                {
                    I2cStat |= I2C1_ACK_TX;
                    REG_WRITE(REG_I2C1_CONFIG, I2cStat); //Set STO
                }
                else
                {
                    I2cStat &= ~I2C1_ACK_TX;
                    REG_WRITE(REG_I2C1_CONFIG, I2cStat);

                    REG_WRITE(REG_I2C1_CONFIG, I2C1_STO);
                    // gi2c1.TransDone = 1;
                    rt_completion_done(&ack);
                    //dbg_log(DBG_LOG, "%s:%d\n", __FUNCTION__, __LINE__);
                }
            }
        }

        I2cStat &= ~I2C1_SI;
        REG_WRITE(REG_I2C1_CONFIG, I2cStat); //Clear SI
    }
}

static void i2c1_enable_interrupt(void)
{
    UINT32 param;
    param = (IRQ_I2C1_BIT);
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);
}

static void i2c1_power_up(void)
{
    UINT32 param;
    param = PWD_I2C1_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);
}

static void i2c1_gpio_config(void)
{
    UINT32 param;

    param = GFUNC_MODE_I2C1;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &param);
}

static void i2c1_set_freq_div(UINT32 div)
{
    UINT32 reg_addr = REG_I2C1_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val = (reg_val & ~(I2C1_FREQ_DIV_MASK << I2C1_FREQ_DIV_POSI)) | ((div & I2C1_FREQ_DIV_MASK) << I2C1_FREQ_DIV_POSI);
    REG_WRITE(reg_addr, reg_val);
}

static void i2c1_hardware_init(void)
{
    /* register interrupt */
    intc_service_register(IRQ_I2C1, PRI_IRQ_I2C1, i2c1_isr);

    /* clear all setting */
    REG_WRITE(REG_I2C1_CONFIG, 0);
}

static void rt_hw_i2c1_init(void)
{
    i2c1_hardware_init();

    i2c1_set_freq_div(I2C_CLK_DIVID(I2C_BAUD_100KHZ));

    i2c1_enable_interrupt();
    i2c1_power_up();
    i2c1_gpio_config();
}

static rt_size_t bk_i2c_master_xfer(struct rt_i2c_bus_device *bus,
                                 struct rt_i2c_msg msgs[],
                                 rt_uint32_t num)
{
    UINT32 reg;
    I2C_OP_ST i2c_op;
    GLOBAL_INT_DECLARATION();

    struct rt_i2c_msg *msg;
    uint32_t i;
    rt_size_t ret = 0;

    dbg_log(DBG_LOG, "%s msg:%d\n", __FUNCTION__, num);

    // each msg.
    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];

        if (!(msg->flags & RT_I2C_NO_START))
        {
            // Start command
        }

        if (msg->flags & RT_I2C_RD)
        {
            // read cycle, read the data.
            GLOBAL_INT_DISABLE();
            gi2c1.TxMode = 0;
            gi2c1.RemainNum = msg->len;
            gi2c1.RegAddr = 0xAA;
            gi2c1.pData = (UINT8 *)msg->buf;
            gi2c1.TransDone = 0;
            GLOBAL_INT_RESTORE();

            reg = ((msg->addr & 0x7f) << 1) + 1; //SET LSB 1
            REG_WRITE(REG_I2C1_DAT, reg);

            reg = REG_READ(REG_I2C1_CONFIG);
            reg |= I2C1_STA | I2C1_TX_MODE | I2C1_ENSMB; //Set STA & TXMODE
            REG_WRITE(REG_I2C1_CONFIG, reg);

            // while (!gi2c1.TransDone);
            rt_completion_wait(&ack, RT_WAITING_FOREVER);
        }
        else
        {
            GLOBAL_INT_DISABLE();
            gi2c1.TxMode = 1;
            gi2c1.RemainNum = msg->len;
            gi2c1.RegAddr = 0xAA;
            gi2c1.pData = (UINT8 *)msg->buf;
            gi2c1.TransDone = 0;
            GLOBAL_INT_RESTORE();

            reg = ((msg->addr & 0x7f) << 1) + 0; //SET LSB 0
            REG_WRITE(REG_I2C1_DAT, reg);

            reg = REG_READ(REG_I2C1_CONFIG);
            reg |= I2C1_STA | I2C1_TX_MODE | I2C1_ENSMB; //Set STA & TXMODE
            REG_WRITE(REG_I2C1_CONFIG, reg);

            // while (!gi2c1.TransDone);
            rt_completion_wait(&ack, RT_WAITING_FOREVER);
        }
    }
    ret = i;

_exit:
    dbg_log(DBG_LOG, "send stop condition\n");

    return ret;
}

static rt_size_t bk_i2c_slave_xfer(struct rt_i2c_bus_device *bus,
                                 struct rt_i2c_msg msgs[],
                                 rt_uint32_t num)
{
    return 0;
}

static rt_err_t bk_i2c_bus_control(struct rt_i2c_bus_device *bus,
                                   rt_uint32_t cmd,
                                   rt_uint32_t arg)
{
    dbg_log(DBG_LOG, "%s cmd:%d\n", __FUNCTION__, cmd);
    return RT_ERROR;
}

static const struct rt_i2c_bus_device_ops bk_i2c_ops =
{
    bk_i2c_master_xfer,
    bk_i2c_slave_xfer,
    bk_i2c_bus_control,
};

static int drv_iic_init(void)
{
    struct rt_i2c_bus_device *i2c_bus;

    i2c_bus = &i2c_bus1;
    rt_hw_i2c1_init();

    i2c_bus->ops = &bk_i2c_ops;
    rt_completion_init(&ack);

    return rt_i2c_bus_device_register(i2c_bus, "i2c1");
}
INIT_DEVICE_EXPORT(drv_iic_init);

#endif /* RT_USING_I2C */
