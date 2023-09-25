/**
 ****************************************************************************************
 *
 * @file hci_uart.h
 *
 * @brief HCIH EIF transport module header file.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef HCI_UART_H_
#define HCI_UART_H_

/**
 ****************************************************************************************
 * @addtogroup HCI_UART HCIH EIF
 * @ingroup HCI
 * @brief HCIH EIF module.
 *
 * This module creates the abstraction between UART driver and HCI generic functions
 * (designed for any transport layer).
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>

/*
 * DEFINES
 ****************************************************************************************
 */
///HCI Transport Header length - change if different transport
#define HCI_TRANSPORT_HDR_LEN                       0x01

///UART header: command message type
#define HCI_CMD_MSG_TYPE                            0x01

///UART header: event message type
#define HCI_EVT_MSG_TYPE                            0x04

/// HCI command packets
//#define HCI_NB_CMD_PKTS            1

///Maximum size for the command parameters
#define HCI_MAX_CMD_LENGTH         255

///Maximum size for the events
#define HCI_MAX_EVT_LENGTH         32

/// Offset of the event header in the buffer
#define HCI_EVT_HDR_OFFSET         1

///Maximum number of commands in ROM
#define HCI_CMD_OPCODE_NB_MAX      12



/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


////HCI TX states
//enum HCI_TX_STATE
//{
//    ///HCI TX Start State - when packet is ready to be sent
//    HCI_STATE_TX_ONGOING,
//    ///HCI TX Done State - TX ended with no error
//    HCI_STATE_TX_IDLE
//};

////HCI RX states
//enum HCI_RX_STATE
//{
//    ///HCI RX Start State - receive message type
//    HCI_STATE_RX_START,
//    ///HCI RX Header State - receive message header
//    HCI_STATE_RX_HDR,
//    ///HCI RX Header State - receive (rest of) message payload
//    HCI_STATE_RX_PAYL,
//    ///HCI RX Header State - receive error
//    HCI_STATE_RX_ERR
//};

///HCI Command header components structure
struct hci_uart_cmd_hdr
{
    ///OGF - command group code
    uint8_t ogf;
    ///OCF - command index in group
    uint8_t ocf;
    ///Flag for opcode known or not at its reception in LL
    uint8_t known_opcode;
    ///Parameter length - the number of bytes of the command parameters
    uint8_t parlen;
};
#if  1
///HCI Environment context structure
struct hci_uart_env_tag
{
    ///Rx state - can be receiving message type, header, payload or error
    uint8_t  rx_state;
    ///Latest received message type: CMD/EVT/ACL.
    uint8_t  curr_msg_type;
    ///Latest received message header, 4 byte buffer for other configuration.
    uint8_t  curr_hdr_buff[4];
    ///Pointer to space reserved for received payload.
    uint8_t *curr_payl_buff;
    ///Space reserved for command reception, with params
    uint8_t  cmd_buf[50/*HCI_MAX_CMD_LENGTH*/];
    ///Space reserved for event transmission, with params
    uint8_t  evt_buf[50/*HCI_MAX_EVT_LENGTH*/];
    ///Last command header received
    struct   hci_uart_cmd_hdr chdr;
};

#endif

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
///HCI environment structure external global variable declaration
extern struct hci_uart_env_tag hci_uart_env;

extern const uint16_t hci_cmd_opcodes[HCI_CMD_OPCODE_NB_MAX];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief HCIH EIF transport initialization.
 *
 * Puts the UART driver in reception, waiting for simple 1 byte message type. Space for
 * reception is allocated with kernel_msg_alloc and the pointer is handed to uart_env.rx. RX
 * interrupt is enabled.
 *****************************************************************************************
 */
void hci_uart_init(void);

/**
 ****************************************************************************************
 * @brief HCIH EIF write function.
 * @param[in] buf   Pointer to the message to be transmitted.
 * @param[in] len   Byte length of buffer to write.
 *****************************************************************************************
 */
void hci_uart_write(uint8_t *buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief Actions after UART TX.
 *
 * Analyzes the status value and sets the hci environment state to TX_DONE/ERR
 * accordingly. This allows the higher function calling uart_write to have feedback
 * and decide the following action (repeat/abort tx in case of error, continue otherwise).
 *
 * @param[in]  status UART Tx status: ok or error.
 *****************************************************************************************
 */
void hci_uart_tx_done(int status);

/**
 ****************************************************************************************
 * @brief Function called at each RX interrupt.
 *
 * According to HCI RX state, the received data is treated differently: message type,
 * header or payload. Once payload is obtained (if existing) the appropriate hci unpacking
 * function is called thus generating a kernel_msg for the appropriate task.
 *
 * @param[in]  status UART RX status: ok or error
 *****************************************************************************************
 */
void hci_uart_rx_done(int status);




/**
****************************************************************************************
* @brief HCI initialization function: initializes states and transport.
*****************************************************************************************
*/
void hci_init(void);

/**
 ****************************************************************************************
 * @brief Function called in HCIH EIF reception routine to dispatch a KE message for a
 *        command. Uses the hci_env command header OGF and OCF components.
 * @param[in]  payl Pointer to receiver buffer payload
 *****************************************************************************************
 */
void hci_cmd_dispatch(uint8_t * payl);

/**
 ****************************************************************************************
 * @brief Trigger sending of HCI event
 *
 *  Function called to trigger sending of HCI event in current event buffer.
 *
 * @param[in]  Length of HCI event
 *****************************************************************************************
 */
void hci_push(uint8_t length);

/// @} HCI_UART

#endif // HCI_UART_H_
