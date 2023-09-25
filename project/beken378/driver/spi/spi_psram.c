#include "include.h"
#include "arm_arch.h"

#include "typedef.h"
#include "sys_config.h"

#include "spi_pub.h"
#include "gpio_pub.h"
#include "sys_ctrl_pub.h"

#include "drv_model_pub.h"
#include "mem_pub.h"
#include "bk_rtos_pub.h"

#if CFG_USE_SPI_MST_PSRAM

#if !CFG_USE_SPI_MASTER
#error "SPI PSRAM NEED CFG_USE_SPI_MASTER ENABLE!!!"
#endif

#define PSRAM_CMD_SET_BURST               0xC0
#define PSRAM_CMD_READ_ID                 0x9F
#define PSRAM_ID_LEN                      11
#define PSRAM_CMD_READ                    0x03
#define PSRAM_CMD_WRITE                   0x02

static void spi_psram_init_extral_gpio(void)
{
    bk_gpio_config_output(SPI_FLASH_WP_GPIO_NUM);
    bk_gpio_output(SPI_FLASH_WP_GPIO_NUM, GPIO_INT_LEVEL_HIGH);

    bk_gpio_config_output(SPI_FLASH_HOLD_GPIO_NUM);
    bk_gpio_output(SPI_FLASH_HOLD_GPIO_NUM, GPIO_INT_LEVEL_HIGH);
}

static void spi_psram_enable_voltage(void)
{
    UINT32 param;

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);

    param = QSPI_IO_3_3V;
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_IO_VOLTAGE, &param);

    param = PSRAM_VDD_3_3V_DEF;
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_VDDRAM_VOLTAGE, &param);
}

static void spi_psram_disable_voltage(void)
{
    UINT32 param;

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_DISABLE, &param);
}

static void spi_psram_send_command(UINT8 cmd)
{
    UINT8 ucmd = cmd;
    struct spi_message msg;

    os_memset(&msg, 0, sizeof(struct spi_message));

    msg.send_buf = &ucmd;
    msg.send_len = sizeof(ucmd);
    msg.recv_buf = NULL;
    msg.recv_len = 0;
    bk_spi_master_xfer(&msg);
}

int32_t spi_psram_init(void)
{
    spi_psram_enable_voltage();
    spi_psram_init_extral_gpio();
    
    return bk_spi_master_init(SPI_DEF_CLK_HZ, SPI_DEF_MODE);
}

int32_t spi_psram_burst_set(uint32_t burst_size)
{
    spi_psram_send_command(PSRAM_CMD_SET_BURST);

    return 0;
}

int32_t spi_psram_read_id(uint8_t id[12])
{
    UINT8 id_buf[PSRAM_ID_LEN] = {0};
    UINT8 id_cmd[] = {PSRAM_CMD_READ_ID};
      
    struct spi_message msg;

    os_memset(id_buf, 0, PSRAM_ID_LEN * sizeof(UINT8));
    os_memset(&msg, 0, sizeof(struct spi_message));
    msg.send_buf = id_cmd;
    msg.send_len = sizeof(id_cmd);
    msg.recv_buf = id_buf;
    msg.recv_len = PSRAM_ID_LEN;
    
    bk_spi_master_xfer(&msg);

    BK_SPI_PRT("MF ID:%02x\r\n", id_buf[3]);
    BK_SPI_PRT("KGD:%02x, (0x5D-pass, 0x55-fail)\r\n", id_buf[4]);

    if(id)
        os_memcpy(id, id_buf, PSRAM_ID_LEN);
    
    return 0;
}

uint32_t spi_psram_read(uint32_t addr, uint8_t* buffer, uint32_t size)
{
    struct spi_message msg;
    UINT8 ucmd[] = {PSRAM_CMD_READ, 0x00, 0x00, 0x00};
    UINT32 send_len;

    if(buffer == NULL)
        return 1;
    
    if(size == 0)
        return 0;
    
    os_memset(&msg, 0, sizeof(struct spi_message));
    ucmd[1] = ((addr >> 16) & 0xff);
    ucmd[2] = ((addr >> 8) & 0xff);
    ucmd[3] = (addr & 0xff);

    msg.send_buf = ucmd;
    msg.send_len = sizeof(ucmd);
    
    msg.recv_buf = buffer;
    msg.recv_len = size;
    
    bk_spi_master_xfer(&msg);
    
    return 0;
}

uint32_t spi_psram_write(uint32_t addr, uint8_t* buffer, uint32_t size)
{
    struct spi_message msg;
    UINT8 *ucmd;
    UINT32 send_len;

    if(buffer == NULL)
        return 1;

    if(size == 0)
        return 0;

    ucmd = os_malloc(size + 4);
    if(!ucmd)
        return 1;  
    
    os_memset(&msg, 0, sizeof(struct spi_message));
    os_memset(ucmd, 0, size + 4);

    ucmd[0] = PSRAM_CMD_WRITE;
    ucmd[1] = ((addr >> 16) & 0xff);
    ucmd[2] = ((addr >> 8) & 0xff);
    ucmd[3] = (addr & 0xff);
    os_memcpy(&ucmd[4], buffer, size);

    msg.send_buf = ucmd;
    msg.send_len = size + 4;
    msg.recv_buf = NULL;
    msg.recv_len = 0;

    bk_spi_master_xfer(&msg);

    os_free(ucmd);
    
    return 0;
}

#endif  // BEKEN_USING_SPI_PSRAM

