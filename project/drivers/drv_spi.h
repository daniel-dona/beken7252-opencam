#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include "typedef.h"
#include "spi.h"
#include "spi_pub.h"
#include "gpio_pub.h"

int rt_hw_spi_bus_register(char *name);
int rt_hw_spi_device_init(void);

#endif
