#ifndef _QSPI_H_
#define _QSPI_H_

#define QSPI_DEBUG

#ifdef 	QSPI_DEBUG
#define QSPI_PRT      os_printf
#define QSPI_WARN     warning_prf
#define QSPI_FATAL    fatal_prf
#else
#define QSPI_PRT      null_prf
#define QSPI_WARN     null_prf
#define QSPI_FATAL    null_prf
#endif

#define QSPI_BASE                           (0x0080D000)
//#define QSPI_DCACHE_BASE					(0x03000000)

#define QSPI_CTRL							(QSPI_BASE + 0x1C * 4)

#define QSPI_DCACHE_WR_CMD					(QSPI_BASE + 0x28 * 4)
#define QSPI_DCACHE_WR_CMD_ENABLE			(0x01 << 0)

#define QSPI_DCACHE_WR_ADDR					(QSPI_BASE + 0x29 * 4)
#define QSPI_DCACHE_WR_ADDR_ENABLE			(0x01 << 0)

#define QSPI_DCACHE_WR_DUM					(QSPI_BASE + 0x2A * 4)

#define QSPI_DCACHE_WR_DAT					(QSPI_BASE + 0x2B * 4)
#define QSPI_DCACHE_WR_DAT_ENABLE			(0x01 << 0)


#define QSPI_DCACHE_RD_CMD					(QSPI_BASE + 0x24 * 4)
#define QSPI_DCACHE_ED_CMD_ENABLE			(0x01 << 0)

#define QSPI_DCACHE_RD_ADDR					(QSPI_BASE + 0x25 * 4)
#define QSPI_DCACHE_RD_ADDR_ENABLE			(0x01 << 0)

#define QSPI_DCACHE_RD_DUM					(QSPI_BASE + 0x26 * 4)

#define QSPI_DCACHE_RD_DAT					(QSPI_BASE + 0x27 * 4)
#define QSPI_DCACHE_RD_DAT_ENABLE			(0x01 << 0)

#define QSPI_DCACHE_REQUEST					(QSPI_BASE + 0x2C * 4)

static UINT32 qspi_open(UINT32 op_flag);
static UINT32 qspi_close(void);
static UINT32 qspi_ctrl(UINT32 cmd, void *param);

#endif //_QSPI_H_
