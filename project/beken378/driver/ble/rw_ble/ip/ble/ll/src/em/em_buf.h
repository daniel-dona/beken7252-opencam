/**
 ****************************************************************************************
 *
 * @file bt_em_buf.h
 *
 * @brief Main API file for the BT EM buffer management system
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef BT_EM_BUF_H_
#define BT_EM_BUF_H_

/**
 ****************************************************************************************
 * @defgroup BT_EM_BUF BT EM buffer management system
 * @ingroup ROOT
 * @brief BT EM buffer management system
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>          // standard integer
#if (BLE_EMB_PRESENT)

#include "common_list.h"        // common list definition
#include "common_bt.h"          // common bt definitions
#include "common_endian.h"      // endianess
#include "reg_ble_em_tx_desc.h"         // EM TX_DESC BLE register
#include "ble_reg_access.h"     // register access
/*
 * DEFINES
 ****************************************************************************************
 */

/// ACL data: handle and data length decoding
#define BT_EM_ACL_DATA_LEN_POS   (0)
#define BT_EM_ACL_DATA_LEN_MASK  (0x03FF)
#define BT_EM_ACL_FLAGS_POS      (12)
#define BT_EM_ACL_FLAGS_MASK     (0xF000)


/*
 * STRUCTURE DEFINITION
 ****************************************************************************************
 */
struct em_buf_node
{
    struct common_list_hdr hdr;
    /// Index of the buffer
    uint16_t idx;
    /// EM buffer pointer
    uint16_t buf_ptr;
};

struct em_desc_node
{
    struct common_list_hdr hdr;
    /// Index of the buffer
    uint16_t idx;
    /// EM Pointer of the buffer
    uint16_t buffer_idx;
    ///  Buffer index
    uint16_t buffer_ptr;
    /// Logical Link Identifier
    uint8_t llid;
    /// Data length
    uint8_t length;
};

/// Tx desc tag
struct em_buf_tx_desc
{
    /// tx pointer
    uint16_t txptr;
    /// tx header
    uint16_t txheader;
    /// tx data pointer
    uint16_t txdataptr;
    /// tx data length extension info
    uint16_t txdle;
};

/// BLE EM buffer management environment structure
struct em_buf_env_tag
{
    /// List of free TX descriptors
    struct common_list tx_desc_free;
    /// List of free TX buffers
    struct common_list tx_buff_free;
    /// Array of TX descriptors (SW tag)
    struct em_desc_node tx_desc_node[BLE_TX_DESC_CNT];
    #if (BLE_TX_BUFFER_CNT > 0)
    /// Array of TX buffer (SW tag)
    struct em_buf_node tx_buff_node[BLE_TX_BUFFER_CNT];
    #endif // (BLE_TX_BUFFER_CNT > 0)
    /// Pointer to TX descriptors
    struct em_buf_tx_desc *tx_desc;
    /// Index of the current RX buffer
    uint8_t rx_current;

};
/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// BLE EM buffer management environment
extern struct em_buf_env_tag em_buf_env;
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize BT EM buffers management system
 ****************************************************************************************
 */
void em_buf_init(void);

/**
 ****************************************************************************************
 * @brief Freeing of a RX buffer
 *
 * @param hdl Handle of the RX buffer to be freed
 *
 ****************************************************************************************
 */
void em_buf_rx_free(uint8_t hdl);

/**
 ****************************************************************************************
 * @brief Returns the pointer to the TX descriptor from the index
 *
 * @return The pointer to the TX descriptor corresponding to the index.
 *
 ****************************************************************************************
 */
  struct em_buf_tx_desc *em_buf_tx_desc_addr_get(uint16_t idx);

/**
 ****************************************************************************************
 * @brief Returns the pointer to the TX node from the index
 *
 * @return The pointer to the TX node corresponding to the index.
 *
 ****************************************************************************************
 */
  struct em_desc_node *em_buf_tx_desc_node_get(uint16_t idx);
/**
 ****************************************************************************************
 * @brief Get the handle of the next RX buffer in the list
 *
 ****************************************************************************************
 */
  uint8_t em_buf_rx_current_get(void);
/**
 ****************************************************************************************
 * @brief Set the current RX buffer
 *
 * @param hdl Handle of the current RX buffer
 *
 ****************************************************************************************
 */
  void em_buf_rx_current_set(uint8_t hdl);

/**
 ****************************************************************************************
 * @brief Get the handle of the next RX buffer in the list
 *
 * @param hdl Handle of the current RX buffer
 *
 ****************************************************************************************
 */
  uint8_t em_buf_rx_next(uint8_t hdl);
/**
 ****************************************************************************************
 * @brief Allocation of a TX data descriptor
 *
 * @return The pointer to the TX descriptor corresponding to the allocated buffer, NULL if
 *         no buffers are available anymore.
 *
 ****************************************************************************************
 */
  struct em_desc_node* em_buf_tx_desc_alloc(void);
/**
 ****************************************************************************************
 * @brief Allocation of a TX data descriptor and a TX data buffer
 *
 * @return The pointer to the TX descriptor corresponding to handle
 *
 ****************************************************************************************
 */
  struct em_buf_node *em_buf_tx_alloc(void);

/**
 ****************************************************************************************
 * @brief Returns the pointer to the TX descriptor from the index
 *
 * @return The pointer to the TX descriptor corresponding to the index.
 *
 ****************************************************************************************
 */
  struct em_buf_tx_desc *em_buf_tx_desc_get(uint16_t idx);
/**
 ****************************************************************************************
 * @brief Freeing of a TX descriptor
 *
 * @param desc  The pointer to the TX descriptor to be freed
 *
 ****************************************************************************************
 */
  void em_buf_tx_desc_free(struct em_desc_node *desc);

#if (BLE_TX_BUFFER_CNT > 0)
/**
 ****************************************************************************************
 * @brief Freeing of a TX buffer
 *
 * @param desc  The pointer to the TX descriptor to be freed
 *
 ****************************************************************************************
 */
  void em_buf_tx_buff_free(int idx);
#endif // (BLE_TX_BUFFER_CNT > 0)

/**
 ****************************************************************************************
 * @brief Returns the pointer to the TX node from the index
 *
 * @return The pointer to the TX node corresponding to the index.
 *
 ****************************************************************************************
 */
  struct em_desc_node *em_buf_node_get(uint16_t idx);
/**
 ****************************************************************************************
 * @brief Return a pointer to a RX Buffer
 *
 * @return The pointer to the RX buffer corresponding to handle.
 *
 ****************************************************************************************
 */
uint8_t *em_buf_rx_buff_addr_get(uint16_t rx_hdl);

/**
 ****************************************************************************************
 * @brief Return a pointer to a TX Buffer
 *
 * @return The pointer to the TX buffer corresponding to handle.
 *
 ****************************************************************************************
 */
uint8_t *em_buf_tx_buff_addr_get(struct em_buf_tx_desc *tx_desc);



/**
 ****************************************************************************************
 * @brief Freeing of TX data descriptor and buffer if needed
 *
 * @param desc  The pointer to the TX descriptor to be freed
 *
* @return if the buffer linked to the descriptor has been flushed
 *
 ****************************************************************************************
 */
bool em_buf_tx_free(struct em_desc_node *desc_to_be_freed);

/**
 ****************************************************************************************
 * @brief Read a 8bits value in EM
 *
 * @param[in] em_addr  Exchange memory address
 *
 * @return 8bits value
 *
 ****************************************************************************************
 */
  uint16_t em_read_8(uint16_t em_addr);

/**
 ****************************************************************************************
 * @brief Read a 16bits value in EM
 *
 * @param[in] em_addr  Exchange memory address
 *
 * @return 16bits value
 *
 ****************************************************************************************
 */
  uint16_t em_read_16(uint16_t em_addr);

/**
 ****************************************************************************************
 * @brief Read a 32bits value in EM
 *
 * @param[in] em_addr  Exchange memory address
 *
 * @return 32bits value
 *
 ****************************************************************************************
 */
 uint32_t em_read_32(uint16_t em_addr);

#endif //BLE_EMB_PRESENT
/// @} EM_BUF

#endif // EM_BUF_H_
