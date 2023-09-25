#ifndef __ROM_CALL_FLASH_H_
#define __ROM_CALL_FLASH_H_

#include "gapm_task.h"

#include "rwble_hl.h"
#include "prf.h"
#include "lld_sleep.h"
#include "rwip.h"

#include <string.h>
#include "kernel_msg.h"
#include "kernel_task.h"
#include "kernel_env.h"             // kernel environment
#include "common_bt.h"            // common bt definitions


/*
struct flash_for_rom_func_tag
{
	kernel_task_id_t (*prf_get_id_from_task)(kernel_msg_id_t task);
	kernel_task_id_t (*prf_get_task_from_id)(kernel_msg_id_t id);
	
	void (*prf_init)(bool reset);
	void (*prf_create)(uint8_t conidx);
	void (*prf_cleanup)(uint8_t conidx, uint8_t reason);
	
	uint8_t (*prf_add_profile)(struct gapm_profile_task_add_cmd * params, kernel_task_id_t* prf_task);
	void (*rwble_hl_reset)(void);
	
	void (*rwip_reset)(void);
	void (*lld_sleep_init)(void);
	
	void (*rwip_prevent_sleep_set)(uint16_t prv_slp_bit);
	void (*rwip_prevent_sleep_clear)(uint16_t prv_slp_bit);
	
	void (*platform_reset)(uint32_t error);
	
};
*/
struct rom_env_tag
{
	kernel_task_id_t (*prf_get_id_from_task)(kernel_msg_id_t task);
	kernel_task_id_t (*prf_get_task_from_id)(kernel_msg_id_t id);
	
	void (*prf_init)(bool reset);
	void (*prf_create)(uint8_t conidx);
	void (*prf_cleanup)(uint8_t conidx, uint8_t reason);
	
	uint8_t (*prf_add_profile)(struct gapm_profile_task_add_cmd * params, kernel_task_id_t* prf_task);
	void (*rwble_hl_reset)(void);
	
	void (*rwip_reset)(void);
	void (*lld_sleep_init)(void);
	
	void (*rwip_prevent_sleep_set)(uint16_t prv_slp_bit);
	void (*rwip_prevent_sleep_clear)(uint16_t prv_slp_bit);
	
	void (*platform_reset)(uint32_t error);
	
	
	uint8_t (*Read_Uart_Buf)(void);
	
};




	

void rom_env_init(struct rom_env_tag *api);

//extern struct flash_for_rom_func_tag RcallF_func_list;
extern struct rom_env_tag rom_env;


void lld_bdaddr_init(const struct  bd_addr *addr);















#endif // __ROM_CALL_FLASH_H_

