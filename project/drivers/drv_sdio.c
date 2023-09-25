#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#define DBG_ENABLE
#define DBG_COLOR

#define DBG_SECTION_NAME    "[drv.sdio] "
#define DBG_LEVEL           DBG_WARNING
#include "rtdbg.h"

#ifdef RT_USING_SDIO

#include <drivers/mmcsd_core.h>

#include "include.h"
#include "sdio_driver.h"
#include "rtos_pub.h"
#include "intc_pub.h"
#include "sys_rtos.h"
#include "sys_ctrl_pub.h"
#include "arm_arch.h"
#include "drv_model_pub.h"
#include "general_dma_pub.h"

#define SDIO_DMA_CHANNEL       GDMA_CHANNEL_5

struct beken_sdio
{
    struct rt_mmcsd_host *host;
    struct rt_mmcsd_req *req;
    struct rt_mmcsd_cmd *cmd;
    uint32_t current_status;
    uint32_t dma_buf[512 / 4]; /* word align */
};
static struct beken_sdio beken_sdio;

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const void *ptr1, rt_size_t buflen)
{
    const rt_uint8_t *ptr = (const rt_uint8_t *)ptr1;
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;

    for (i = 0; i < buflen; i += 16)
    {
        rt_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%02X ", buf[i + j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        rt_kprintf("\n");
    }
}

rt_inline int is_valid_dma(const void *buf)
{
    uint32_t addr = (uint32_t)buf;

    if (((addr & 3) == 0) && (addr < 0x00440000) && (addr >= 0x00400000))
    {
        LOG_D("buf 0x%08X is valid DMA.", addr);
        return 1;
    }

    LOG_D("buf 0x%08X is invalid DMA.", addr);
    return 0;
}

static void sdio_dma_finish_handler(UINT32 flag)
{
    GDMA_CFG_ST en_cfg;
    struct rt_mmcsd_req *req = beken_sdio.req;
    struct rt_mmcsd_cmd *cmd = beken_sdio.cmd;

    LOG_I("sdio_dma_finish_handler flag:0x%08X", flag);

    en_cfg.channel = SDIO_DMA_CHANNEL;
    en_cfg.param = 0;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
}

static void sdio_dma_rx(void *buf, uint32_t len)
{
    GDMACFG_TPYES_ST cfg;
    GDMA_CFG_ST en_cfg;

    memset(&cfg, 0, sizeof(GDMACFG_TPYES_ST));
    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    cfg.dstdat_width = 32;
    cfg.srcdat_width = 32;

    cfg.srcptr_incr = 0;
    cfg.dstptr_incr = 1;

    cfg.src_start_addr = (void*)REG_SDCARD_RD_DATA_ADDR;
    cfg.dst_start_addr = buf;

    cfg.channel = SDIO_DMA_CHANNEL;
    cfg.prio = 0;
    cfg.u.type5.dst_loop_start_addr = buf;
    cfg.u.type5.dst_loop_end_addr = buf + len; // TODO: ??

    cfg.half_fin_handler = NULL;
    cfg.fin_handler = sdio_dma_finish_handler;

    cfg.src_module = GDMA_X_SRC_SDIO_RX_REQ;
    cfg.dst_module = GDMA_X_DST_DTCM_WR_REQ;

    sddev_control(GDMA_DEV_NAME, CMD_GDMA_CFG_TYPE5, &cfg);

    en_cfg.channel = SDIO_DMA_CHANNEL;
    en_cfg.param = len;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_TRANS_LENGTH, &en_cfg);

#if 1
    en_cfg.channel = SDIO_DMA_CHANNEL;
    en_cfg.param = 1;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
#endif    
}

static void sdio_dma_tx(void *buf, uint32_t len)
{
    GDMACFG_TPYES_ST cfg;
    GDMA_CFG_ST en_cfg;

    memset(&cfg, 0, sizeof(GDMACFG_TPYES_ST));
    memset(&en_cfg, 0, sizeof(GDMA_CFG_ST));

    cfg.dstdat_width = 32;
    cfg.srcdat_width = 32;

    cfg.srcptr_incr = 1;
    cfg.dstptr_incr = 0;

    cfg.src_start_addr = buf;
    cfg.dst_start_addr = (void*)REG_SDCARD_WR_DATA_ADDR;

    cfg.channel = SDIO_DMA_CHANNEL;
    cfg.prio = 0;
    cfg.u.type4.src_loop_start_addr = buf;
    cfg.u.type4.src_loop_end_addr = buf + len;

    cfg.half_fin_handler = NULL;
    cfg.fin_handler = sdio_dma_finish_handler;

    cfg.src_module = GDMA_X_SRC_DTCM_RD_REQ;
    cfg.dst_module = GDMA_X_DST_SDIO_TX_REQ;

    sddev_control(GDMA_DEV_NAME, CMD_GDMA_CFG_TYPE4, &cfg);

    en_cfg.channel = SDIO_DMA_CHANNEL;
    en_cfg.param = len;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_TRANS_LENGTH, &en_cfg);

#if 1
    en_cfg.channel = SDIO_DMA_CHANNEL;
    en_cfg.param = 1;
    sddev_control(GDMA_DEV_NAME, CMD_GDMA_SET_DMA_ENABLE, &en_cfg);
#endif    
}

/*
sd host interface rate:
0x0: 1 division of clock frequency, 26M/1=26Mhz.
0x1: 2 division of clock frequency, 26M/2=13Mhz.
0x2: 4 division of clock frequency, 26M/4=6.5M.
0x3: 128 division of clock frequency, 26M/128=203Khz.
 */
static void sdio_set_clk_div(uint32_t sdio_clk)
{
    UINT32 reg;
    uint8_t clkdiv = 3;

    if (sdio_clk >= (1000 * 1000 * 25)) /* 25M ==> 26M */
    {
        clkdiv = 1;//TODO: 
    }
    else if (sdio_clk >= (1000 * 1000 * 13))
    {
        clkdiv = 1;
    }
    else if (sdio_clk >= (1000 * 6500))
    {
        clkdiv = 2;
    }
    LOG_D("sdio clkdiv=%d", clkdiv)

    reg = REG_READ(REG_SDCARD_FIFO_THRESHOLD);
    reg &= ~(SDCARD_FIFO_SD_RATE_SELECT_MASK << SDCARD_FIFO_SD_RATE_SELECT_POSI);
    reg |= ((clkdiv & SDCARD_FIFO_SD_RATE_SELECT_MASK)
            << SDCARD_FIFO_SD_RATE_SELECT_POSI);
    REG_WRITE(REG_SDCARD_FIFO_THRESHOLD, reg);
}

static void mmc_request_send(struct rt_mmcsd_host *host, struct rt_mmcsd_req *req)
{
    struct beken_sdio *sdio = (struct beken_sdio *)host->private_data;
    struct rt_mmcsd_cmd *cmd = req->cmd;
    rt_tick_t tick_start, tick_end;
    uint32_t flags = 0;

    sdio->host = host;
    sdio->req = req;
    sdio->cmd = req->cmd;

    int cmd_resp_type;
    cmd_resp_type = resp_type(req->cmd);
    LOG_D("CMD%d, arg: 0x%08X, resp_type:%d", req->cmd->cmd_code, req->cmd->arg, cmd_resp_type);

    if(req->data)
    {
        struct rt_mmcsd_data *data = req->data;

        LOG_D("have data, blksize:%d, blks:%d, flags: 0x%08X, buf: 0x%08X", req->data->blksize, req->data->blks, req->data->flags, data->buf);

        if (data->flags & DATA_DIR_READ)
        {
            if (is_valid_dma(data->buf))
            {
                sdio_dma_rx(data->buf, req->data->blksize);
            }
            else
            {
                if (data->blksize > sizeof(sdio->dma_buf))
                {
                    LOG_E("blksize %d too big, dma buf size=%d.", data->blksize);
                    cmd->err = -RT_EINVAL;
                    mmcsd_req_complete(host);
                    return;
                }
                sdio_dma_rx(sdio->dma_buf, req->data->blksize);
            }

            sdio_set_data_timeout(DEF_HIGH_SPEED_CMD_TIMEOUT); // TODO: DEF_DATA_TIME_OUT
            sdio_setup_data((req->data->flags & DATA_DIR_WRITE) ? SDIO_WR_DATA : SDIO_RD_DATA, data->blksize);
        }

        if (data->flags & DATA_DIR_WRITE)
        {
            uint32_t reg;

            LOG_D("DATA_DIR_WRITE.");
            reg = REG_READ(REG_SDCARD_FIFO_THRESHOLD);
            REG_WRITE(REG_SDCARD_FIFO_THRESHOLD, reg | SDCARD_FIFO_SD_STA_RST);

            reg &= (0xffff | (SDCARD_FIFO_SD_RATE_SELECT_MASK << SDCARD_FIFO_SD_RATE_SELECT_POSI));
            reg |= (0x0101 | SDCARD_FIFO_TX_FIFO_RST);
            REG_WRITE(REG_SDCARD_FIFO_THRESHOLD, reg);

            // prepare data.
            //dump_hex(data->buf, data->blksize);
            memcpy(sdio->dma_buf, data->buf, data->blksize);
            for(uint32_t i=0; i<data->blksize/4; i++)
            {
                uint32_t swap = sdio->dma_buf[i];
                sdio->dma_buf[i] = ((swap >> 24) | ((swap >> 8) & 0xff00) |
                                    ((swap << 8) & 0xff0000) | ((swap << 24) & 0xff000000));
            }
        }
    }

    if (resp_type(cmd) == RESP_NONE)
    {
        flags = SD_CMD_NORESP;
    }
    else
    {
        /* set 136 bit response for R2, 48 bit response otherwise */
        if (resp_type(cmd) == RESP_R2)
            flags = SD_CMD_LONG;
        else
            flags = SD_CMD_SHORT;
    }

    sdio_sendcmd_function(req->cmd->cmd_code, flags,
                          DEF_LOW_SPEED_CMD_TIMEOUT, //sdio_cmd_ptr->timeout,
                          (void *)req->cmd->arg);
}

static void mmc_set_iocfg(struct rt_mmcsd_host *host, struct rt_mmcsd_io_cfg *io_cfg)
{
    LOG_I("mmc_set_iocfg clock:%d, bus_width:%d, power_mode:%d", io_cfg->clock, io_cfg->bus_width, io_cfg->power_mode);

    if (io_cfg->power_mode == MMCSD_POWER_ON)
    {
        sdio_set_clk_div(io_cfg->clock);
        sdio_clk_config(1);
    }
    else
    {
        sdio_clk_config(0);
    }

    if (io_cfg->bus_width != MMCSD_BUS_WIDTH_1)
    {
        LOG_E("only support 1bit.");
    }
}

static int sdio_receive_data(uint8_t *receive_buf, int size)
{
    uint32_t reg, i;

    for (i = 0; i < size;)
    {
        while (1)
        {
            //software  needn't handle dead-loop,hardware can guarantee.
            if (REG_READ(REG_SDCARD_FIFO_THRESHOLD) & SDCARD_FIFO_RXFIFO_RD_READY)
                break;
        }

        reg = REG_READ(REG_SDCARD_RD_DATA_ADDR);
        *(receive_buf + i++) = reg & 0xff;
        *(receive_buf + i++) = (reg >> 8) & 0xff;
        *(receive_buf + i++) = (reg >> 16) & 0xff;
        *(receive_buf + i++) = (reg >> 24) & 0xff;
    }

    return i;
}

static int sdio_send_data(const uint8_t *send_buf, int size)
{
    return size;
}

static void sdio_isr(void)
{
    uint32_t status;
    struct beken_sdio *sdio = &beken_sdio;
    struct rt_mmcsd_req *req = sdio->req;
    struct rt_mmcsd_cmd *cmd = sdio->cmd;

    status = REG_READ(REG_SDCARD_CMD_RSP_INT_SEL);

    LOG_D("sdio_isr, status: 0x%08X", status);

    if(status & SD_CMD_RSP)
    {
        // CMD0 0x00000081, SDCARD_CMDRSP_NORSP_END_INT
        // CMD8 0x00020482, SDCARD_CMDRSP_RSP_END_INT SDCARD_CMDRSP_CMD_CRC_OK
        // CMD5 0x00020084, SDCARD_CMDRSP_TIMEOUT_INT
        // ACMD41 0x000FC882, R3, SDCARD_CMDRSP_CMD_CRC_FAIL
        // CMD51 0x000CC482,
        // CMD17 0x00044482, 
        
        uint32_t SD_RSP_INDEX = (status >> 14) & 0x3F;
        //LOG_D("CMD%d, SD_CMD_RSP, SD_RSP_INDEX:%d", req->cmd->cmd_code, SD_RSP_INDEX);

        if (status & SDCARD_CMDRSP_TIMEOUT_INT)
        {
            if(cmd->cmd_code == 5)
            {
                LOG_I("CMD%d, SDCARD_CMDRSP_TIMEOUT_INT", req->cmd->cmd_code);
            }
            else
            {
                LOG_W("CMD%d, SDCARD_CMDRSP_TIMEOUT_INT", req->cmd->cmd_code);
            }

            cmd->err = -RT_ETIMEOUT;
            mmcsd_req_complete(beken_sdio.host);
        }

        if (status & SDCARD_CMDRSP_CMD_CRC_FAIL)
        {
            int type = resp_type(cmd);

            if ((type == RESP_R3) || (type == RESP_R4))
            {
                LOG_I("CMD%d, resp_type:%d, skip resp crc error!", req->cmd->cmd_code, type);
                status |= SDCARD_CMDRSP_CMD_CRC_OK; // force return resp.
            }
            else
            {
                LOG_W("CMD%d, SDCARD_CMDRSP_CMD_CRC_FAIL", req->cmd->cmd_code);
                cmd->err = -RT_ERROR;
                mmcsd_req_complete(beken_sdio.host);
            }
        }

        if ((status & SDCARD_CMDRSP_NORSP_END_INT) && (resp_type(req->cmd) == RESP_NONE))
        {
            LOG_D("CMD%d, RESP_NONE complete", req->cmd->cmd_code);
            cmd->err = 0;
            mmcsd_req_complete(beken_sdio.host);
        }

        if( (status & SDCARD_CMDRSP_RSP_END_INT) && (status & SDCARD_CMDRSP_CMD_CRC_OK) )
        {
            cmd->resp[0] = REG_READ(REG_SDCARD_CMD_RSP_AGUMENT0);
            cmd->resp[1] = REG_READ(REG_SDCARD_CMD_RSP_AGUMENT1);
            cmd->resp[2] = REG_READ(REG_SDCARD_CMD_RSP_AGUMENT2);
            cmd->resp[3] = REG_READ(REG_SDCARD_CMD_RSP_AGUMENT3);

            //LOG_D("CMD%d, SD_RSP_INDEX:%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X", req->cmd->cmd_code, SD_RSP_INDEX,
            //      cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);

            cmd->err = 0;
            if (!cmd->data)
            {
                cmd->err = 0;
                mmcsd_req_complete(beken_sdio.host);
            }
            else
            {
                struct rt_mmcsd_data *data = cmd->data;

                if ((cmd->data) && (cmd->data->flags & DATA_DIR_WRITE))
                {
                    sdio_dma_tx(sdio->dma_buf, req->data->blksize);

                    sdio_set_data_timeout(DEF_HIGH_SPEED_CMD_TIMEOUT);
                    sdio_setup_data((req->data->flags & DATA_DIR_WRITE) ? SDIO_WR_DATA : SDIO_RD_DATA, req->data->blksize);
                }
            }
        }
    }

    if (status & SD_DATA_RSP)
    {
        LOG_D("SD_DATA_RSP");

        if (cmd->data)
        {
            struct rt_mmcsd_data *data = cmd->data;

            if (status & SDCARD_CMDRSP_DATA_TIME_OUT_INT)
            {
                LOG_E("SDCARD_CMDRSP_DATA_TIME_OUT_INT");
                cmd->err = -RT_ETIMEOUT;
                mmcsd_req_complete(beken_sdio.host);
            }
            else if (status & SDCARD_CMDRSP_DATA_CRC_FAIL)
            {
                LOG_E("SDCARD_CMDRSP_DATA_CRC_FAIL");
                cmd->err = -RT_ERROR;
                mmcsd_req_complete(beken_sdio.host);
            }
            else if( (status & SDCARD_CMDRSP_DATA_REC_END_INT) && (data->flags & DATA_DIR_READ) )
            {
                LOG_D("SDCARD_CMDRSP_DATA_REC_END_INT & DATA_DIR_READ");

                if(!is_valid_dma(data->buf))
                {
                    memcpy(data->buf, sdio->dma_buf, data->blksize);
                }
                //dump_hex(data->buf, data->blksize);

                cmd->err = 0;
                mmcsd_req_complete(beken_sdio.host);
            }
            else if( (status & SDCARD_CMDRSP_DATA_WR_END_INT) && (data->flags & DATA_DIR_WRITE) )
            {
                LOG_D("SDCARD_CMDRSP_DATA_WR_END_INT & DATA_DIR_WRITE");

                cmd->err = 0;
                mmcsd_req_complete(beken_sdio.host);
            }
            else
            {
                LOG_E("unkown status");
            }
            
        }
    }

    REG_WRITE(REG_SDCARD_CMD_RSP_INT_SEL, status); // W1C
}

static void sdio_hw_init(void)
{
    /* config sdcard gpio */
    sdio_gpio_config();

    /* reset sdcard moudle register */
    sdio_register_reset();

    /* set sdcard low clk */
    sdio_set_low_clk();

    /* set sdcard  clk enable*/
    sdio_clk_config(1);

    intc_service_register(IRQ_SD, PRI_IRQ_SD, sdio_isr);
    intc_enable(IRQ_SD);
    REG_WRITE(REG_SDCARD_CMD_RSP_INT_MASK, 0x17F);
}

static const struct rt_mmcsd_host_ops ops = 
{
    mmc_request_send,
    mmc_set_iocfg,
    RT_NULL,
    RT_NULL,
};

static struct rt_mmcsd_host *curr_host;
static int beken_sdio_init(void)
{
    struct rt_mmcsd_host *host = RT_NULL;

    host = mmcsd_alloc_host();
    if (!host)
    {
        LOG_E("alloc host failed");
        goto err;
    }

    sdio_hw_init();

    host->ops = &ops;
    host->freq_min = 1000 * 204; /* 26/128=203.125Khz */
    host->freq_max = 1000 * 1000 * 26;
    host->valid_ocr = VDD_32_33 | VDD_33_34;
    host->flags = MMCSD_MUTBLKWRITE | MMCSD_SUP_HIGHSPEED | MMCSD_SUP_SDIO_IRQ;
    // host->flags |= MMCSD_BUSWIDTH_4;
    host->max_seg_size = 512;
    host->max_dma_segs = 4;
    host->max_blk_size = 512;
    host->max_blk_count = 32;

    host->private_data = &beken_sdio;

    mmcsd_change(host);

    return RT_EOK;

err:
    if (host)
        rt_free(host);

    return -RT_EIO;
}
INIT_ENV_EXPORT(beken_sdio_init);

#endif /* RT_USING_SDIO */
