
#ifndef __I2C_H__
#define __I2C_H__


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>        // standard definition
#include "BK_HCI_Protocol.h"
#include "BK3435_reg.h"




#define I2C_CONFIG_I2C_ENABLE_POSI    31
#define I2C_CONFIG_I2C_ENABLE_MASK    (0x01UL << I2C_CONFIG_I2C_ENABLE_POSI)
#define I2C_CONFIG_I2C_ENABLE_SET     (0x01UL << I2C_CONFIG_I2C_ENABLE_POSI)

#define I2C_CONFIG_SMBTOE_POSI         28
#define I2C_CONFIG_SMBTOE_MASK         (0x01UL << I2C_CONFIG_SMBTOE_POSI)
#define I2C_CONFIG_SMBTOE_SET          (0x01UL << I2C_CONFIG_SMBTOE_POSI)

#define I2C_CONFIG_SMBFTE_POSI         29
#define I2C_CONFIG_SMBFTE_MASK         (0x01UL << I2C_CONFIG_SMBFTE_POSI)
#define I2C_CONFIG_SMBFTE_SET          (0x01UL << I2C_CONFIG_SMBFTE_POSI)

#define I2C_CONFIG_INH_POSI            30
#define I2C_CONFIG_INH_MASK            (0x01UL << I2C_CONFIG_INH_POSI)
#define I2C_CONFIG_INH_CLEAR           (0x00UL << I2C_CONFIG_INH_POSI)
#define I2C_CONFIG_INH_SET             (0x01UL << I2C_CONFIG_INH_POSI)

#define I2C_CONFIG_IDLE_CR_POSI        0
#define I2C_CONFIG_IDLE_CR_MASK        (0x07UL << I2C_CONFIG_IDLE_CR_POSI)
#define I2C_CONFIG_IDLE_CR_DEFAULT     (0x03UL << I2C_CONFIG_IDLE_CR_POSI)

#define I2C_CONFIG_SCL_CR_POSI         3
#define I2C_CONFIG_SCL_CR_MASK         (0x07UL << I2C_CONFIG_SCL_CR_POSI)
#define I2C_CONFIG_SCL_CR_DEFAULT      (0x07UL << I2C_CONFIG_SCL_CR_POSI)

#define I2C_CONFIG_FREQ_DIV_POSI       6
#define I2C_CONFIG_FREQ_DIV_MASK       (0x03FFUL << I2C_CONFIG_FREQ_DIV_POSI)

#define I2C_CONFIG_SLAVE_ADDR_POSI     16
#define I2C_CONFIG_SLAVE_ADDR_MASK     (0x03FFUL << I2C_CONFIG_SLAVE_ADDR_POSI)

#define I2C_CONFIG_CLOCK_SEL_POSI      26
#define I2C_CONFIG_CLOCK_SEL_MASK      (0x03UL << I2C_CONFIG_CLOCK_SEL_POSI)
#define I2C_CONFIG_CLOCK_SEL_TIMER0    (0x00UL << I2C_CONFIG_CLOCK_SEL_POSI)
#define I2C_CONFIG_CLOCK_SEL_TIMER1    (0x01UL << I2C_CONFIG_CLOCK_SEL_POSI)
#define I2C_CONFIG_CLOCK_SEL_TIMER2    (0x02UL << I2C_CONFIG_CLOCK_SEL_POSI)
#define I2C_CONFIG_CLOCK_SEL_FREQ_DIV  (0x03UL << I2C_CONFIG_CLOCK_SEL_POSI)

#define I2C_STATUS_INT_MODE_POSI       6
#define I2C_STATUS_INT_MODE_MASK       (0x03UL << I2C_STATUS_INT_MODE_POSI)

#define I2C_STATUS_ACK_POSI            8
#define I2C_STATUS_ACK_MASK            (0x01UL << I2C_STATUS_ACK_POSI)
#define I2C_STATUS_ACK_SET             (0x01UL << I2C_STATUS_ACK_POSI)

#define I2C_STATUS_STOP_POSI           9
#define I2C_STATUS_STOP_MASK           (0x01UL << I2C_STATUS_STOP_POSI)
#define I2C_STATUS_STOP_SET            (0x01UL << I2C_STATUS_STOP_POSI)

#define ICU_I2C_CLK_PWD_CLEAR()         do {REG_AHB0_ICU_I2CCLKCON = 0x00;} while (0)
#define ICU_I2C_CLK_PWD_SET()           do {REG_AHB0_ICU_I2CCLKCON = 0x01;} while (0)


#define I2C_DEFAULT_SLAVE_ADDRESS      (0x72)

#define I2C_ENABLE						0


void i2c_init(uint32_t slaveAddr, uint32_t baudRate);
STATUS i2c_write(uint8_t devAddr, uint8_t addr, uint8_t*buf, uint8_t size);
STATUS i2c_read(uint8_t devAddr, uint8_t addr, uint8_t*buf, uint8_t size);
void i2c_isr(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif      /* __I2C_H__ */


