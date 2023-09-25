/**
 ****************************************************************************************
 *
 * @file app_wlist.h
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef APP_WLIST_H_
#define APP_WLIST_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (BLE_APP_PRESENT)

#include <stdint.h>          // Standard Integer Definition
#include "common_bt.h"           // Common BT Definitions
#include "architect.h"            // Platform Definitions
#include "gapc.h"            // GAPC Definitions
#include "gapm_task.h"

#if (NVDS_SUPPORT)
#include "nvds.h"
#endif // (NVDS_SUPPORT)

/*
 * DEFINES
 ****************************************************************************************
 */
/// Application environment
extern struct app_env_tag app_env;

void app_cfg_test(void);



int app_gapm_white_list_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_white_list_size_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id);
																
																
int app_gapm_ral_list_size_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_ral_size_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id);
																
																
int app_gapm_ral_addr_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_ral_addr_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id);
														
																
void appm_gapm_resolv_dev_addr(bd_addr_t addr,struct gap_sec_key irk);
																
int app_gapm_resolv_dev_addr_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_addr_solved_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id);
																
void appm_gapm_gen_rand_addr(void);
																
int app_gapm_gen_rand_addr_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_dev_bdaddr_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id);
																
void appm_gapm_gen_rand_number(void);																
																
int app_gapm_gen_rand_number_ind_handler(kernel_msg_id_t const msgid,
                                struct gapm_gen_rand_nb_ind const *param,
                                kernel_task_id_t const dest_id,
                                kernel_task_id_t const src_id);
																
																
								
void appm_gapm_set_irk(struct gap_sec_key irk) ;



/// @} APP

#endif //(BLE_APP_PRESENT)

#endif // APP_H_
