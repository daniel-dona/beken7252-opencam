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

#include <rtdevice.h>

#if CFG_USE_SPI_MST_FLASH

#if !CFG_USE_SPI_MASTER
#error "SPI FLASH NEED CFG_USE_SPI_MASTER ENABLE!!!"
#endif

#define FLASH_PHY_PAGE_SIZE       256
#define FLASH_PHY_SECTOR_SIZE     4096
#define FLASH_PHY_BLK_32K         (32*1024)
#define FLASH_PHY_BLK_64K         (64*1024)

#define CMD_READ_ID               0x9F
#define READ_ID_RESPONE_LEN       3

#define CMD_READ_STATUS_S7_0      0x05
#define CMD_READ_STATUS_S15_8     0x35
#define READ_STATUS_LEN           1
#define FLASH_STATUS_WIP_BIT      (1 << 0)
#define FLASH_STATUS_WEL_BIT      (1 << 1)
#define FLASH_STATUS_PRETECT_MASK (0x3F << 2)

#define CMD_WRITE_STATUS          0x01

#define ERASE_MODE_ALL            0x01
#define ERASE_MODE_BLOCK_64K      0x02
#define ERASE_MODE_BLOCK_32K      0x03
#define ERASE_MODE_SECTOR         0x04

#define CMD_ERASE_ALL             0xc7  // 0x60
#define CMD_ERASE_BLK_64K         0xD8
#define CMD_ERASE_BLK_32K         0x52
#define CMD_ERASE_SECTOR          0x20

#define DELAY_WHEN_BUSY_MS        10

#define CMD_READ_DATA             0x03
#define CMD_PAGE_PROG             0x02

#define CMD_WRITE_ENABLE          0x06
#define CMD_WRITE_DISABLE         0x04

static void spi_flash_send_command(UINT8 cmd)
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

static UINT16 spi_flash_read_status(void)
{
    UINT16 ustatus = 0;
    UINT8 ustatus_buf[READ_STATUS_LEN] = {0};
    UINT8 ustatus_cmd[] = {CMD_READ_STATUS_S7_0};
    struct spi_message msg;

    os_memset(&msg, 0, sizeof(struct spi_message));
    msg.send_buf = ustatus_cmd;
    msg.send_len = sizeof(ustatus_cmd);
    msg.recv_buf = ustatus_buf;
    msg.recv_len = READ_STATUS_LEN;
    bk_spi_master_xfer(&msg);
    ustatus = ustatus_buf[0];

    ustatus_buf[0] = 0;
    ustatus_cmd[0] = CMD_READ_STATUS_S15_8;
    bk_spi_master_xfer(&msg);
    ustatus |= (ustatus_buf[0]) << 8;

    //BK_SPI_PRT("uid:%04x\r\n", ustatus);

    return ustatus;
}

static UINT32 spi_flash_is_busy(void)
{
    UINT8 ustatus_buf[READ_STATUS_LEN] = {0};
    UINT8 ustatus_cmd[] = {CMD_READ_STATUS_S7_0};
    struct spi_message msg;

    os_memset(&msg, 0, sizeof(struct spi_message));
    msg.send_buf = ustatus_cmd;
    msg.send_len = sizeof(ustatus_cmd);
    msg.recv_buf = ustatus_buf;
    msg.recv_len = READ_STATUS_LEN;
    bk_spi_master_xfer(&msg);
    
    ustatus_buf[0];

    return (ustatus_buf[0] & FLASH_STATUS_WIP_BIT);
}

static void spi_flash_write_status(UINT16 ustatus)
{
    UINT8 ustatus_cmd[] = {CMD_WRITE_STATUS, 0x00, 0x00};
    struct spi_message msg;

    os_memset(&msg, 0, sizeof(struct spi_message));
    ustatus_cmd[1] = ustatus & 0xff;
    ustatus_cmd[2] = (ustatus >> 8) & 0xff;

    msg.send_buf = ustatus_cmd;
    msg.send_len = sizeof(ustatus_cmd);
    msg.recv_buf = NULL;
    msg.recv_len = 0;

    while(spi_flash_is_busy())
    {
        bk_rtos_delay_milliseconds(DELAY_WHEN_BUSY_MS);
    }

    spi_flash_send_command(CMD_WRITE_ENABLE); 

    while(spi_flash_is_busy())
    {
        bk_rtos_delay_milliseconds(DELAY_WHEN_BUSY_MS);
    }
    
    bk_spi_master_xfer(&msg);
}

static void spi_flash_earse(UINT32 addr, UINT32 mode)
{
    struct spi_message msg;
    UINT8 ucmd[] = {0x00, 0x00, 0x00, 0x00};
    UINT32 send_len;

    os_memset(&msg, 0, sizeof(struct spi_message));

    if(mode == ERASE_MODE_ALL)
    {
        ucmd[0] = CMD_ERASE_ALL;
        send_len = 1;
    }
    else 
    {
        if(mode == ERASE_MODE_BLOCK_64K)
        {
            ucmd[0] = CMD_ERASE_BLK_64K;
        }
        else if(mode == ERASE_MODE_BLOCK_32K)
        {
            ucmd[0] = CMD_ERASE_BLK_32K;
        }
        else if(mode == ERASE_MODE_SECTOR)
        {
            ucmd[0] = CMD_ERASE_SECTOR;
        }
        else
        {
            BK_SPI_FATAL("earse wrong mode:%d\r\n", mode);
            return;
        }
        
        ucmd[1] = ((addr >> 16) & 0xff);
        ucmd[2] = ((addr >> 8) & 0xff);
        ucmd[3] = (addr & 0xff);
        send_len = 4;
    }
    
    msg.send_buf = ucmd;
    msg.send_len = send_len;

    msg.recv_buf = NULL;
    msg.recv_len = 0;

    while(spi_flash_is_busy())
    {
        bk_rtos_delay_milliseconds(DELAY_WHEN_BUSY_MS);
    }

    spi_flash_send_command(CMD_WRITE_ENABLE); 

    while(spi_flash_is_busy())
    {
        bk_rtos_delay_milliseconds(DELAY_WHEN_BUSY_MS);
    }
    
    bk_spi_master_xfer(&msg);
}

static int spi_flash_read_page(UINT32 addr, UINT32 size, UINT8 *dst)
{
    struct spi_message msg;
    UINT8 ucmd[] = {CMD_READ_DATA, 0x00, 0x00, 0x00};
    UINT32 send_len;

    if(dst == NULL)
        return 1;

    if(size > FLASH_PHY_PAGE_SIZE)
        return 1;
    
    if(size == 0)
        return 0;
    
    os_memset(&msg, 0, sizeof(struct spi_message));
    ucmd[1] = ((addr >> 16) & 0xff);
    ucmd[2] = ((addr >> 8) & 0xff);
    ucmd[3] = (addr & 0xff);

    msg.send_buf = ucmd;
    msg.send_len = sizeof(ucmd);
    
    msg.recv_buf = dst;
    msg.recv_len = size;
    
    while(spi_flash_is_busy())
    {
        bk_rtos_delay_milliseconds(DELAY_WHEN_BUSY_MS);
    }

    bk_spi_master_xfer(&msg);
    
    return 0;
}

static int spi_flash_program_page(UINT32 addr, UINT32 size, UINT8 *src)
{
    struct spi_message msg;
    UINT8 *ucmd;
    UINT32 send_len;

    if(src == NULL)
        return 1;

    if(size > FLASH_PHY_PAGE_SIZE)
        return 1;

    if(size == 0)
        return 0;

    ucmd = os_malloc(size + 4);
    if(!ucmd)
        return 1;  
    
    os_memset(&msg, 0, sizeof(struct spi_message));
    os_memset(ucmd, 0, size + 4);

    ucmd[0] = CMD_PAGE_PROG;
    ucmd[1] = ((addr >> 16) & 0xff);
    ucmd[2] = ((addr >> 8) & 0xff);
    ucmd[3] = (addr & 0xff);
    os_memcpy(&ucmd[4], src, size);

    msg.send_buf = ucmd;
    msg.send_len = size + 4;
    msg.recv_buf = NULL;
    msg.recv_len = 0;

    while(spi_flash_is_busy())
    {
        bk_rtos_delay_milliseconds(DELAY_WHEN_BUSY_MS);
    }
    
    spi_flash_send_command(CMD_WRITE_ENABLE); 

    while(spi_flash_is_busy())
    {
        bk_rtos_delay_milliseconds(DELAY_WHEN_BUSY_MS);
    }
    
    bk_spi_master_xfer(&msg);

    os_free(ucmd);
    
    return 0;
}

static void spi_flash_init_extral_gpio(void)
{
    bk_gpio_config_output(SPI_FLASH_WP_GPIO_NUM);
    bk_gpio_output(SPI_FLASH_WP_GPIO_NUM, GPIO_INT_LEVEL_HIGH);

    bk_gpio_config_output(SPI_FLASH_HOLD_GPIO_NUM);
    bk_gpio_output(SPI_FLASH_HOLD_GPIO_NUM, GPIO_INT_LEVEL_HIGH);
}

static void spi_flash_enable_voltage(void)
{
    UINT32 param;

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_ENABLE, &param);

    param = QSPI_IO_3_3V;
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_IO_VOLTAGE, &param);

    param = PSRAM_VDD_3_3V_DEF;
    sddev_control(SCTRL_DEV_NAME, CMD_QSPI_VDDRAM_VOLTAGE, &param);
}

static void spi_flash_disable_voltage(void)
{
    UINT32 param;

    param = BLK_BIT_MIC_QSPI_RAM_OR_FLASH;
    sddev_control(SCTRL_DEV_NAME, CMD_SCTRL_BLK_DISABLE, &param);
}

/////////////////////////////////////////////////////
// spi flash extern interface
int spi_flash_init(void)
{
    spi_flash_enable_voltage();
    spi_flash_init_extral_gpio();
    
    return bk_spi_master_init(SPI_DEF_CLK_HZ, SPI_DEF_MODE);
}

void spi_flash_deinit(void)
{
    spi_flash_disable_voltage();
    bk_spi_master_deinit();
}

UINT32 spi_flash_read_id(void)
{
    UINT32 uid = 0;
    UINT8 uid_buf[READ_ID_RESPONE_LEN] = {0};
    UINT8 uid_cmd[] = {CMD_READ_ID};
    struct spi_message msg;

    os_memset(&msg, 0, sizeof(struct spi_message));
    msg.send_buf = uid_cmd;
    msg.send_len = sizeof(uid_cmd);
    msg.recv_buf = uid_buf;
    msg.recv_len = READ_ID_RESPONE_LEN;
    
    bk_spi_master_xfer(&msg);

    uid = (uid_buf[0] << 16) | (uid_buf[1] << 8) | (uid_buf[2]);

    BK_SPI_PRT("uid:%06x\r\n", uid);

    return uid;
}

int spi_flash_read(UINT32 addr, UINT32 size, UINT8 *dst)
{
    struct spi_message msg;
    UINT8 ucmd[] = {CMD_READ_DATA, 0x00, 0x00, 0x00};
    UINT32 send_len;

    if(dst == NULL)
        return 1;
    
    if(size == 0)
    {
        return 0;
    }
    
    for(int i=0; i<size; )
    {
        int ret;
        UINT32 dsize;

        if((size - i) >= FLASH_PHY_PAGE_SIZE)
            dsize = FLASH_PHY_PAGE_SIZE;
        else
            dsize = size - i;
        
        ret = spi_flash_read_page(addr, dsize, dst);
        if(ret)
        {
            BK_SPI_PRT("spiff read page err:%d\r\n", ret);
            return 1;
        }
        
        addr = addr + dsize;
        dst = dst + dsize;
        i = i + dsize;
    }
    
    return 0;
}

int spi_flash_write(UINT32 addr, UINT32 size, UINT8 *src)
{
    if(src == NULL)
        return 1;
    
    if(size == 0)
    {
        return 0;
    }
        
    for(int i=0; i<size; )
    {
        int ret;
        UINT32 dsize;

        if((size - i) >= FLASH_PHY_PAGE_SIZE)
            dsize = FLASH_PHY_PAGE_SIZE;
        else
            dsize = size - i;
        
        ret = spi_flash_program_page(addr, dsize, src);
        if(ret)
        {
            BK_SPI_PRT("spiff write page err:%d\r\n", ret);
            return 1;
        }
        
        addr = addr + dsize;
        src = src + dsize;
        i = i + dsize;
    }
    
    return 0;
}

int spi_flash_erase(UINT32 addr, UINT32 size)
{
    int left_size = (int)size;

    while (left_size > 0)
    {
        UINT32 erase_size = 0, erase_mode;
        
        if(left_size <= 4 * 1024)
        {
            erase_size = 4 * 1024;
            erase_mode = ERASE_MODE_SECTOR;
        }
        else if(size <= 32 * 1024)
        {
            erase_size = 32 * 1024;
            erase_mode = ERASE_MODE_BLOCK_32K;
        }
        else
        {
            erase_size = 64 * 1024;
            erase_mode = ERASE_MODE_BLOCK_64K;
        }

        spi_flash_earse(addr, erase_mode);
        if(addr & (erase_size - 1))
        {
            size = erase_size - (addr & (erase_size - 1));
        }
        else
        {
            size = erase_size;
        }
            
        left_size -= size;
        addr += size;
    }
    
    return 0;
}

void spi_flash_protect(void)
{
    spi_flash_write_status(0x003C);
}

void spi_flash_unprotect(void)
{
    spi_flash_write_status(0x0000);
}
#endif  // BEKEN_USING_SPI_FLASH

