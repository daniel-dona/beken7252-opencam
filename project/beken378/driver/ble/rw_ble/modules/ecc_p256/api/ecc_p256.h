/**
 ****************************************************************************************
 *
 * @file ecc_p256.h
 *
 * @brief  ECC functions for P256
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef ECC_P256_H_
#define ECC_P256_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if (SECURE_CONNECTIONS)

#include <stdint.h>
#include <stdbool.h>
#include "kernel_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

/// Multiplication result message structure
struct ecc_result_ind
{
    uint8_t key_res_x[32];
    uint8_t key_res_y[32];
};


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Initialize Elliptic Curve algorithm
 ****************************************************************************************
 */
void ecc_init(bool reset);

/**
 ****************************************************************************************
 * @brief Generate a Secret Key compliant with ECC P256 algorithm
 *
 * If key is forced, just check its validity
 *
 * @param[out] secret_key Private key - MSB First
 * @param[in]  forced True if provided key is forced, else generate it.
 ****************************************************************************************
 */
void ecc_gen_new_secret_key(uint8_t* secret_key, bool forced);

/**
 ****************************************************************************************
 * @brief Generate a new Public key pair using ECC P256 algorithm
 *
 * @param[in] secret_key Private key - MSB First
 * @param[in] blocking   Force to run full algorithm without continue mode
 ****************************************************************************************
 */
void ecc_gen_new_public_key(uint8_t* secret_key256, kernel_msg_id_t msg_id, kernel_task_id_t task_id);

/**
 ****************************************************************************************
 * @brief Generate a new DHKey using ECC P256 algorithm
 *
 * @param[in] secret_key Private key                  - MSB First
 * @param[in] pub_key_x  Peer public key x coordinate - LSB First
 * @param[in] pub_key_y  Peer public key y coordinate - LSB First
 * @param[in] msg_id     Message task ID for the result indication
 * @param[in] task_id    Client task ID (Task type + instance)
 ****************************************************************************************
 */
void ecc_generate_key256(const uint8_t* secret_key, const uint8_t* public_key_x, const uint8_t* public_key_y, kernel_msg_id_t msg_id, kernel_task_id_t task_id);

/**
 ****************************************************************************************
 * @brief Abort a current DHKey generation procedure
 *
 * @param[in] task_id    Client task ID (Task type + instance)
 ****************************************************************************************
 */
void ecc_abort_key256_generation(kernel_task_id_t task_id);

/**
 ****************************************************************************************
 * @brief Retrieve debug private and public keys
 *
 * @param[out] secret_key Private key             - MSB First
 * @param[out] pub_key_x  Public key x coordinate - LSB First
 * @param[out] pub_key_y  Public key y coordinate - LSB First
 ****************************************************************************************
 */
void ecc_get_debug_Keys(uint8_t*secret_key, uint8_t* pub_key_x, uint8_t* pub_key_y);
#endif // (SECURE_CONNECTIONS)


#endif /* ECC_P256_H_ */
