/**
 ****************************************************************************************
 *
 * @file uart.h
 *
 * @brief UART Driver for HCI over UART operation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _UART_H_
#define _UART_H_

/**
 ****************************************************************************************
 * @defgroup UART UART
 * @ingroup DRIVERS
 * @brief UART driver
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions
#include "rwip_config.h"


#if UART_PRINTF_EN

#if  !BLE_TESTER
#define UART_PRINTF	uart_printf_null  
#else
#define UART_PRINTF uart_printf  
#endif //!BLE_TESTER

#else
#define UART_PRINTF uart_printf_null 
#endif // #if UART_PRINTF_EN
 

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes the UART to default values.
 *****************************************************************************************
 */
void uart_init(uint32_t baudrate);

void dbg_initial(void);

void uart_clear_rxfifo(void);

uint8_t Uart_Read_Byte(void);
uint8_t Read_Uart_Buf(void);

int dbg_putchar(char * st);
int uart_putchar(char * st);
int uart_printf(const char *fmt,...);
int uart_printf_null(const char *fmt,...);
int dbg_printf(const char *fmt,...);
void uart_print_int(unsigned int num);
uint8_t check_uart_stop(void);

void cpu_delay( volatile unsigned int times);


/****** REG  ****************/
void uart_send(unsigned char *buff, int len);
void TRAhcit_UART_Rx(void);

#define UART0_RX_FIFO_MAX_COUNT  128

#define UART0_TX_FIFO_MAX_COUNT  128

extern unsigned char uart_rx_buf[UART0_RX_FIFO_MAX_COUNT];
extern unsigned char uart_tx_buf[UART0_TX_FIFO_MAX_COUNT];
extern volatile bool uart_rx_done ;
extern volatile unsigned long uart_rx_index ;
/****** REG  ****************/



#ifndef CFG_ROM
/**
 ****************************************************************************************
 * @brief Enable UART flow.
 *****************************************************************************************
 */
void uart_flow_on(void);

/**
 ****************************************************************************************
 * @brief Disable UART flow.
 *****************************************************************************************
 */
bool uart_flow_off(void);
#endif //CFG_ROM

/**
 ****************************************************************************************
 * @brief Finish current UART transfers
 *****************************************************************************************
 */
void uart_finish_transfers(void);

/**
 ****************************************************************************************
 * @brief Starts a data reception.
 *
 * @param[out] bufptr   Pointer to the RX buffer
 * @param[in]  size     Size of the expected reception
 * @param[in]  callback Pointer to the function called back when transfer finished
 * @param[in]  dummy    Dummy data pointer returned to callback when reception is finished
 *****************************************************************************************
 */
void uart_read(uint8_t *bufptr, uint32_t size, void (*callback) (void*, uint8_t), void* dummy);

/**
 ****************************************************************************************
 * @brief Starts a data transmission.
 *
 * @param[in] bufptr   Pointer to the TX buffer
 * @param[in] size     Size of the transmission
 * @param[in] callback Pointer to the function called back when transfer finished
 * @param[in] dummy    Dummy data pointer returned to callback when transmission is finished
 *****************************************************************************************
 */
void uart_write(uint8_t *bufptr, uint32_t size, void (*callback) (void*, uint8_t), void* dummy);

#if defined(CFG_ROM)
/**
 ****************************************************************************************
 * @brief Poll UART on reception and transmission.
 *
 * This function is used to poll UART for reception and transmission.
 * It is used when IRQ are not used to detect incoming bytes.
 *****************************************************************************************
 */
void uart_poll(void);
#endif //CFG_ROM

/**
 ****************************************************************************************
 * @brief Serves the data transfer interrupt requests.
 *
 * It clears the requests and executes the appropriate callback function.
 *****************************************************************************************
 */
void uart_isr(void);


/// @} UART
#endif /* _UART_H_ */
