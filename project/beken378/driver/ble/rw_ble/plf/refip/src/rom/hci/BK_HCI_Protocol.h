/*************************************************************
 * File Name:     BK_HCI_Protocol.h
 * Author:        GuWenFu
 * Created Date:  @2016-04-14
 * Description:   Header file of BK_HCI_Protocol.c
 *
 * History:       2016-04-14 gwf    create this file
 */

#ifndef __BK_HCI_PROTOCOL_H__

#define __BK_HCI_PROTOCOL_H__


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


//#include "config.h"
//#include "BK_System.h"

	typedef unsigned char       BYTE;
typedef signed   char       int8;       // 有符号8位整型变量
typedef signed   short      int16;      // 有符号16位整型变量
typedef signed   long       int32;      // 有符号32位整型变量
typedef signed   long long  int64;      // 有符号64位整型变量
typedef unsigned char       uint8;      // 无符号8位整型变量
typedef unsigned short      uint16;     // 无符号16位整型变量
typedef unsigned long       uint32;     // 无符号32位整型变量
typedef unsigned long long  uint64;     // 无符号64位整型变量
typedef float               fp32;       // 单精度浮点数(32位长度)
typedef double              fp64;       // 双精度浮点数(64位长度)

typedef signed   char       s8;
typedef signed   short      s16;
typedef signed   long       s32;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned long       u32;
//typedef unsigned long       u_int32;

typedef unsigned char  		UINT8;      /* Unsigned  8 bit quantity        */
typedef signed   char  		INT8;       /* Signed    8 bit quantity        */
typedef unsigned short 		UINT16;     /* Unsigned 16 bit quantity        */
typedef signed   short 		INT16;      /* Signed   16 bit quantity        */
typedef unsigned int   		UINT32;     /* Unsigned 32 bit quantity        */
typedef signed   int   		INT32;      /* Signed   32 bit quantity        */
typedef unsigned long long  UINT64;		/* Unsigned 32 bit quantity        */
typedef signed   long long  INT64;		/* Signed   32 bit quantity        */
typedef float         		FP32;		/* Single precision floating point */
typedef double         		FP64;		/* Double precision floating point */
typedef unsigned int        size_t;
typedef unsigned char       BOOLEAN;



typedef enum        // by gwf
{
    OK = 0,
    ERROR = -1
} STATUS;

typedef enum        // by gwf
{
    NO = 0,
    YES = 1
} ASK;


#define HCI_EVENT_HEAD_LENGTH       0x03
#define HCI_COMMAND_HEAD_LENGTH     0x04

/*
 * HCI transport type bytes
 */
enum {
    TRA_HCIT_COMMAND = 1,
    TRA_HCIT_ACLDATA = 2,
    TRA_HCIT_SCODATA = 3,
    TRA_HCIT_EVENT   = 4
};


#define VENDOR_SPECIFIC_DEBUG_OGF   0x3F
#define BEKEN_OCF                   0xE0
#define HCI_COMMAND_COMPLETE_EVENT  0x0E

enum {
    LINK_CHECK_CMD              = 0x00,
    REGISTER_WRITE_CMD          = 0x01,
    REGISTER_READ_CMD           = 0x03,

    // Bluetooth Tx/Rx Test 
    TX_PN9_CMD                  = 0xA0,
    RX_PN9_CMD                  = 0xA1,
    CEVA_BR_SEND_CMD            = 0xA2,
    CEVA_BR_RECV_CMD            = 0xA3,
    BLE_ADVERTISER_CMD          = 0xA4,
    CEVA_FHS_SEND_CMD           = 0xA5,

    // Beken Digital Test
    TEST_CMD_CHIP_ID            = 0xB0,
    TEST_CMD_RF_TXRX            = 0xB1,
    TEST_CMD_DEEPSLEEP          = 0xB2,
    TEST_CMD_SUBDEEPSLEEP       = 0xB3,
    TEST_CMD_DIGITAL_OP         = 0xB4,
    TEST_CMD_DMIPS              = 0xB5,
    TEST_CMD_BR_TXRX            = 0xB6,
    TEST_CMD_LE_TXRX            = 0xB7,
    TEST_CMD_BK24_TXRX          = 0xB8,
    TEST_CMD_LPO_BT_LINK        = 0xB9,
    TEST_CMD_ADC                = 0xBA,
    TEST_CMD_FLASH_OP           = 0xBB,
    TEST_CMD_SPI                = 0xBC,
    TEST_CMD_I2C                = 0xBD,
    TEST_CMD_WDT                = 0xBE,
    TEST_CMD_3DS                = 0xBF,

    LOOP_MODE_CMD               = 0xCC,
} ;//BEKEN_CMD;


#define __PACKED_POST__  __attribute__((packed))

typedef struct {
    u8 code;             /**< 0x01: HCI Command Packet
                              0x02: HCI ACL Data Packet
                              0x03: HCI Synchronous Data Packet
                              0x04: HCI Event Packet */
    struct {
        u16 ocf : 10;    /**< OpCode Command Field */
        u16 ogf : 6;     /**< OpCode Group Field */
    } __PACKED_POST__ opcode;
    u8 total;
    u8 cmd;              /**< private command */
    u8 param[];
} __PACKED_POST__ HCI_COMMAND_PACKET;

typedef struct {
    u8 code;             /**< 0x01: HCI Command Packet
                              0x02: HCI ACL Data Packet
                              0x03: HCI Synchronous Data Packet
                              0x04: HCI Event Packet */
    u8 event;            /**< 0x00-0xFF: Each event is assigned a 1-Octet event code used to uniquely identify different types of events*/
    u8 total;            /**< Parameter Total Length */
    u8 param[];
} __PACKED_POST__ HCI_EVENT_PACKET;

typedef struct {
    u32 addr;
    u32 value;
} __PACKED_POST__ REGISTER_PARAM;


extern void TRAhcit_UART_Rx(void);


#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif      /* #ifndef __BK_HCI_PROTOCOL_H__ */
