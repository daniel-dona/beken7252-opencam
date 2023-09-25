#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "[IIC2]"
#define DBG_LEVEL DBG_INFO
// #define DBG_LEVEL         DBG_INFO
// #define DBG_LEVEL         DBG_WARNING
// #define DBG_LEVEL         DBG_ERROR
#define DBG_COLOR
#include <rtdbg.h>

#define I2C_CLK (0)
#define I2C_DAT (1)

#if defined(RT_USING_I2C) && defined(RT_USING_I2C2)
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

typedef struct i2c2_msg
{
    UINT8 TxMode; //0: Read;  1: Write
    UINT8 RegAddr;
    UINT16 RemainNum;
    UINT8 *pData;
    volatile UINT8 TransDone;
} I2C2_MSG_ST, *I2C2_MSG_PTR;

static struct rt_completion ack;
static struct rt_i2c_bus_device i2c_bus2;
static I2C2_MSG_ST gi2c2;

static void i2c2_set_idle_cr(UINT32 idle_cr)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val = (reg_val & ~(I2C2_IDLE_CR_MASK << I2C2_IDLE_CR_POSI)) | ((idle_cr & I2C2_IDLE_CR_MASK) << I2C2_IDLE_CR_POSI);

    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_scl_cr(UINT32 scl_cr)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val = (reg_val & ~(I2C2_SCL_CR_MASK << I2C2_SCL_CR_POSI)) | ((scl_cr & I2C2_SCL_CR_MASK) << I2C2_SCL_CR_POSI);

    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_freq_div(UINT32 div)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val = (reg_val & ~(I2C2_FREQ_DIV_MASK << I2C2_FREQ_DIV_POSI)) | ((div & I2C2_FREQ_DIV_MASK) << I2C2_FREQ_DIV_POSI);

    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_slave_addr(UINT32 addr)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val = (reg_val & ~(I2C2_SLV_ADDR_MASK << I2C2_SLV_ADDR_POSI)) | ((addr & I2C2_SLV_ADDR_MASK) << I2C2_SLV_ADDR_POSI);
    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_smbus_cs(UINT32 cs)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    reg_val = (reg_val & ~(I2C2_SMB_CS_MASK << I2C2_SMB_CS_POSI)) | ((cs & I2C2_SMB_CS_MASK) << I2C2_SMB_CS_POSI);
    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_timeout_en(UINT32 timeout)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (timeout)
        reg_val |= I2C2_SMB_TOE;
    else
        reg_val &= ~I2C2_SMB_TOE;
    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_free_detect(UINT32 free)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (free)
        reg_val |= I2C2_SMB_FTE;
    else
        reg_val &= ~I2C2_SMB_FTE;
    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_salve_en(UINT32 en)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (en)
        reg_val |= I2C2_INH;
    else
        reg_val &= ~I2C2_INH;
    REG_WRITE(reg_addr, reg_val);
}

static void i2c2_set_smbus_en(UINT32 en)
{
    UINT32 reg_addr = REG_I2C2_CONFIG;
    UINT32 reg_val = REG_READ(reg_addr);

    if (en)
        reg_val |= I2C2_ENSMB;
    else
        reg_val &= ~I2C2_ENSMB;
    REG_WRITE(reg_addr, reg_val);
}

/*
To set INT_MODE as 1 1
INT_MODE[6:7]===>1 1
*/
static void i2c2_set_int_mode_en(void)
{
    UINT32 reg_addr = REG_I2C2_STA;
    UINT32 reg_val = REG_READ(reg_addr);
    reg_val |= (1 << 6);
    REG_WRITE(reg_addr, reg_val);

    reg_val = REG_READ(reg_addr);
    reg_val |= (1 << 7);
    REG_WRITE(reg_addr, reg_val);
}

////////////////////////////////////////////////////////////////////////////////
static void i2c2_gpio_config(void)
{
    UINT32 param;

    param = GFUNC_MODE_I2C2;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &param);
}

static void i2c2_power_up(void)
{
    UINT32 param;
    param = PWD_I2C2_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);
}

static void i2c2_power_down(void)
{
    UINT32 param;
    param = PWD_I2C2_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_DOWN, &param);
}

static void i2c2_enable_interrupt(void)
{
    UINT32 param;
    param = (IRQ_I2C2_BIT);
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);
}

static void i2c2_disable_interrupt(void)
{
    UINT32 param;
    param = (IRQ_I2C2_BIT);
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);
}

static void i2c2_isr(void)
{
    UINT32 I2cStat;

    I2cStat = REG_READ(REG_I2C2_STA);
    if (I2cStat & I2C2_SMBUS_SI)
    {
        if (I2cStat & I2C2_SMBUS_STA)
        {
            I2cStat &= ~I2C2_SMBUS_STA; //Clear STA
            REG_WRITE(REG_I2C2_STA, I2cStat);

            if (!(I2cStat & I2C2_SMBUS_ACK))
            {
                I2cStat |= I2C2_SMBUS_STOP;
                REG_WRITE(REG_I2C2_STA, I2cStat); //Set STO
                rt_completion_done(&ack);
                //gi2c2.TransDone = 1;
                //dbg_log(DBG_LOG, "%s:%d\n", __FUNCTION__, __LINE__);
            }
            else
            {
                if (gi2c2.TxMode)
                {
                    //REG_WRITE(REG_I2C2_DAT, gi2c2.RegAddr); //Tx Reg Address
                    REG_WRITE(REG_I2C2_DAT, *(gi2c2.pData++));
                    gi2c2.RemainNum--;
                }
                else
                {

                    //rt_kprintf("i2c2_isr \r\n");
                    //dbg_log(DBG_LOG, "%s:%d\n", __FUNCTION__, __LINE__);
                    //I2cStat &= ~i2c2_TX_MODE;
                    //REG_WRITE(REG_i2c2_CONFIG, I2cStat); //Clear TXMODE
                }
            }
        }
        else
        {
            if (gi2c2.TxMode)
            {
                //Reg Write
                if (!(I2cStat & I2C2_SMBUS_ACK) || (gi2c2.RemainNum == 0))
                {
                    //Rx NACK or All Data TX Done
                    I2cStat |= I2C2_SMBUS_STOP;
                    REG_WRITE(REG_I2C2_STA, I2cStat); //Set STO
                    //gi2c2.TransDone = 1;
                    rt_completion_done(&ack);
                    //dbg_log(DBG_LOG, "%s:%d\n", __FUNCTION__, __LINE__);
                }
                else
                {
                    REG_WRITE(REG_I2C2_DAT, *(gi2c2.pData++));
                    gi2c2.RemainNum--;
                }
            }
            else
            {
                //Reg Read
                *(gi2c2.pData++) = REG_READ(REG_I2C2_DAT);
                gi2c2.RemainNum--;
                if (gi2c2.RemainNum)
                {
                    I2cStat |= I2C2_SMBUS_ACK;
                    REG_WRITE(REG_I2C2_STA, I2cStat); //Set STO
                }
                else
                {
                    I2cStat &= ~I2C2_SMBUS_ACK;
                    REG_WRITE(REG_I2C2_STA, I2cStat);

                    I2cStat |= I2C2_SMBUS_STOP;
                    REG_WRITE(REG_I2C2_STA, I2cStat);
                    rt_completion_done(&ack);
                    //gi2c2.TransDone = 1;
                    //dbg_log(DBG_LOG, "%s:%d\n", __FUNCTION__, __LINE__);
                }
            }
        }
        I2cStat &= ~I2C2_SMBUS_SI;
        REG_WRITE(REG_I2C2_STA, I2cStat); //Clear SI
    }
}

static void i2c2_hardware_init(void)
{
    //UINT32 reg;
    /* register interrupt */
    intc_service_register(IRQ_I2C2, PRI_IRQ_I2C2, i2c2_isr);

    /* clear all setting */
    REG_WRITE(REG_I2C2_CONFIG, 0);
    REG_WRITE(REG_I2C2_STA, 0);
}

static void rt_hw_i2c2_init(void)
{
    i2c2_hardware_init();

    i2c2_set_freq_div(I2C_CLK_DIVID(I2C_BAUD_100KHZ));

    i2c2_set_idle_cr(0x3);
    i2c2_set_scl_cr(0x4);
    i2c2_set_smbus_cs(0x3);
    i2c2_set_timeout_en(1);
    i2c2_set_free_detect(1);
    i2c2_set_slave_addr(0xf);
    i2c2_set_salve_en(1); // diable i2c slave

    i2c2_enable_interrupt();
    i2c2_power_up();
    i2c2_gpio_config();
    i2c2_set_smbus_en(1);
    i2c2_set_int_mode_en();
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
            //Start command
        }

        if (msg->flags & RT_I2C_RD)
        {
            //read cycle , read the data
            GLOBAL_INT_DISABLE();
            gi2c2.TxMode = 0;
            gi2c2.RemainNum = msg->len;
            gi2c2.RegAddr = 0xAA;
            gi2c2.pData = (UINT8 *)msg->buf;
            gi2c2.TransDone = 0;
            GLOBAL_INT_RESTORE();

            reg = ((msg->addr & 0x7f) << 1) + 1; //SET LSB 1
            REG_WRITE(REG_I2C2_DAT, reg);

            reg = REG_READ(REG_I2C2_STA);
            reg |= I2C2_SMBUS_STA; //Set STA & TXMODE
            REG_WRITE(REG_I2C2_STA, reg);
            rt_completion_wait(&ack, RT_WAITING_FOREVER);
        }
        else
        {
            GLOBAL_INT_DISABLE();
            gi2c2.TxMode = 1;
            gi2c2.RemainNum = msg->len;
            gi2c2.RegAddr = 0xAA;
            gi2c2.pData = (UINT8 *)msg->buf;
            gi2c2.TransDone = 0;
            GLOBAL_INT_RESTORE();

            reg = ((msg->addr & 0x7f) << 1) + 0; //SET LSB 0
            REG_WRITE(REG_I2C2_DAT, reg);

            reg = REG_READ(REG_I2C2_STA);
            reg |= I2C2_SMBUS_STA; //Set STA & TXMODE
            REG_WRITE(REG_I2C2_STA, reg);
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

static int drv_iic2_init(void)
{
    struct rt_i2c_bus_device *i2c_bus;

    i2c_bus = &i2c_bus2;
    rt_hw_i2c2_init();

    i2c_bus->ops = &bk_i2c_ops;
    rt_completion_init(&ack);

    return rt_i2c_bus_device_register(i2c_bus, "i2c2");
}

INIT_DEVICE_EXPORT(drv_iic2_init);

#endif /* RT_USING_I2C2 */
