/**
 ****************************************************************************************
 *
 * @file ir.h
 *
 * @brief ir Driver for ir operation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _IR_H_
#define _IR_H_

/**
 ****************************************************************************************
 * @defgroup IR 
 * @ingroup DRIVERS
 * @brief IR driver
 *
 * @{
 *
 ****************************************************************************************
 */
 
#define  IR_SEND  1
#define  IR_RECEIVE  0

#define IR_ENABLE				0

 //=====================================================================================
// IR && RNG
//=====================================================================================

#define APB9_RGN_IR_BASE							  	0x00806900
#define REG_APB9_RNG_CFG               (*((volatile unsigned long *) (APB9_RGN_IR_BASE + 0X0 * 0X04)))
#define REG_APB9_RNG_DATA               (*((volatile unsigned long *) (APB9_RGN_IR_BASE + 0X1 * 0X04)))
#define REG_APB9_IR_CFG               (*((volatile unsigned long *) (APB9_RGN_IR_BASE + 0X2 * 0X04)))

#define XVR_REG0B               					(*((volatile unsigned int *)   (APB_XVER_BASE+0x2C)))

#define IR_DATA_IN_BIT										  (0X04)
#define IR_DATA_OUT_BIT											(0x02)
#define	IR_SEN_BIT												(17)

#define	port_ir_high()		REG_APB5_GPIOA_DATA |= 0x08
#define	port_ir_low()		REG_APB5_GPIOA_DATA &= (~0x08)


//ir_zaibo 代表载波ir_rec_overtimer 代表超时 ir_return_count 代表返回ir_rec_buf 收到的数据
extern	uint32_t	ir_zaibo, ir_rec_overtimer, ir_return_count;
extern	uint32_t	ir_rec_buf[200];


enum ir_ret
{
	IR_REC_ERR =0,
	IR_OVERTIMER =1,
	IR_REC_OK =2,
	
};

//power =0 关IR 电源，power =1  开IR 电源
void			ir_power(uint8_t	 power);
// 红外发射管电流选择 0= 50MA ; 1= 100MA ;2 =200MA 默认是100MA
void 		ir_send_power(uint8_t power);
// 测试学习功能的DEMO 
void 		ir_demo_test(void);
// 接收灵敏度设置，建议只使用ir_sensitivity_regb_set  sen 值0-3   0  灵敏度最高 
void 		ir_sensitivity_regb_set(uint8_t sen);
// 			带调制波发送 zaibo =载波时间( 单位微妙)  send_timer =发射时间澹(单位微妙)
void			ir_modulation_send(uint32_t	zaibo, uint32_t  send_timer);
// 			不带调制波发送   send_timer =发射时间澹(单位微妙)  send_level =0,发射口输出低，send_level =1,发射口输出高
void			ir_no_modulation_send(uint32_t  send_timer , uint32_t  send_level);
//           发送一串数据，zaibo =载波时间( 单位微妙，载波=0代表无载波发送，非0 代表载波周期)，
//			send_buf 发射数据 数组，send_count，发射长度
//			第一个数据带载波，第二个不带，依次发送
void			ir_send(uint32_t	zaibo , uint32_t	*send_buf, uint32_t	send_count);
//          发射接收模式选 =0 接收模式  =1  发射模式
void			ir_mode(uint8_t	mode);
//			接收函数,overtimer =接收超时(单位毫秒) 超时会退出 , zaibo_phase =载波时长 (单位微妙)
//           zaibo_phase =0 代表无载波学习，非0 代表有载波的学习
//          rec_buf = 接收数据   com_count =指定收到多少个高低波形后退出，return_count 退出时收到多少高低波形
uint8_t		ir_receive(uint32_t overtimer,  uint32_t *zaibo_phase ,uint32_t *rec_buf , uint32_t  com_count ,uint32_t  *return_count);




#endif // 

