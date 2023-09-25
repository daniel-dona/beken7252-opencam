#ifndef _SPI_PUB_H_
#define _SPI_PUB_H_

#define SPI_FAILURE                (1)
#define SPI_SUCCESS                (0)

#define SPI_DEV_NAME                "spi"

#define SPI_CMD_MAGIC              (0xe250000)
enum
{
    CMD_SPI_UNIT_ENABLE = SPI_CMD_MAGIC + 1,
    CMD_SPI_SET_MSTEN,
    CMD_SPI_SET_CKPHA,
    CMD_SPI_SET_CKPOL,
    CMD_SPI_SET_BITWIDTH,
    CMD_SPI_SET_NSSID,
    CMD_SPI_SET_CKR,
    CMD_SPI_RXINT_EN,
    CMD_SPI_TXINT_EN,
    CMD_SPI_RXOVR_EN,
    CMD_SPI_TXOVR_EN,
    CMD_SPI_RXFIFO_CLR,
    CMD_SPI_RXINT_MODE,
    CMD_SPI_TXINT_MODE,
    CMD_SPI_INIT_MSTEN,
    CMD_SPI_GET_BUSY,
    CMD_SPI_SET_RX_CALLBACK,
    CMD_SPI_SET_TX_NEED_WRITE_CALLBACK,
    CMD_SPI_SET_TX_FINISH_CALLBACK,
    CMD_SPI_DEINIT,
};

#define BK_SPI_DEBUG                1
#include "uart_pub.h"
#if BK_SPI_DEBUG
#define BK_SPI_PRT               os_printf
#define BK_SPI_WPRT              warning_prf
#define BK_SPI_FATAL             fatal_prf
#else
#define BK_SPI_PRT               null_prf
#define BK_SPI_WPRT              null_prf
#define BK_SPIFATAL             null_prf
#endif

#define USE_SPI_GPIO_14_17          (0)
#define USE_SPI_GPIO_30_33          (1)
#define USE_SPI_GPIO_NUM            USE_SPI_GPIO_14_17
#define SPI_FLASH_WP_GPIO_NUM       (GPIO18)
#define SPI_FLASH_HOLD_GPIO_NUM     (GPIO19)

#define SPI_DEF_CLK_HZ              (10 * 1000 * 1000)
#define TX_FINISH_FLAG              (1 << 31)

#define BK_SPI_CPOL                 0x01
#define BK_SPI_CPHA                 0x02
#define SPI_DEF_MODE                (~((BK_SPI_CPOL)|(BK_SPI_CPHA)))

struct spi_message
{
    void *send_buf;
    UINT32 send_len;
    
    void *recv_buf;
    UINT32 recv_len;
};

typedef void (*spi_callback)(int port, void *param);
struct spi_callback_des
{
    spi_callback callback;
    void  *param;
};

/*******************************************************************************
* Function Declarations
*******************************************************************************/
UINT32 spi_read_rxfifo(UINT8 *data);
UINT32 spi_write_txfifo(UINT8 data);

void spi_init(void);
void spi_exit(void);
void spi_isr(void);
UINT32 spi_ctrl(UINT32 cmd, void *param);

int bk_spi_master_xfer(struct spi_message *msg);
int bk_spi_master_init(UINT32 rate, UINT32 mode);
int bk_spi_master_deinit(void);

int bk_spi_slave_init(UINT32 rate, UINT32 mode);
int bk_spi_slave_xfer(struct spi_message *msg);

#endif //_SPI_PUB_H_
