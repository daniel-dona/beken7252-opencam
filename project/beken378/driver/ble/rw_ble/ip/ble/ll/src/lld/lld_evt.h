/**
 ****************************************************************************************
 *
 * @file lld_evt.h
 *
 * @brief Declaration of functions used for event scheduling
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef LLD_EVT_H_
#define LLD_EVT_H_

/**
 ****************************************************************************************
 * @addtogroup LLDEVT
 * @ingroup LLD
 * @brief Event scheduling functions
 *
 * This module implements the primitives used for event scheduling
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>

#include "common_bt.h"
#include "common_list.h"
#include "common_llcp.h"
#include "rwip_config.h"
#include "reg_blecore.h"
#include "llm.h"
#include "llc_task.h"

/*
 * MACROS
 ****************************************************************************************
 */

/// Get BLE Event environment address from an element
#define LLD_EVT_ENV_ADDR_GET(elt)        \
                ((struct lld_evt_tag *)(&(elt->env)))


/*
 * DEFINES
 ****************************************************************************************
 */

/// Size of the LLD event table. It shall be equal to the max number of supported
/// connections * 2 + 1 for the scanning/advertising
#define LLD_EVT_TAB_SIZE  (BLE_CONNECTION_MAX * 2 + 1)

/// Invalid value for an interval
#define LLD_EVT_INTERVAL_INVALID  0xFFFF

/// Maximum value an interval can take. This value is for time comparison
#define MAX_INTERVAL_TIME  3193600  // Max number of 625us periods for a connection interval

/// Default RX window size
#define LLD_EVT_DEFAULT_RX_WIN_SIZE     (14)
/// Frame Duration
#define LLD_EVT_FRAME_DURATION          (1250)
/// Max Finecounter value
#define LLD_EVT_FINECNT_MAX             (SLOT_SIZE - 1)
/// MAX LP Clock Jitter allowed by the specification (Core 4.0 - vol 6, -B - 4.2.2)
#define LLD_EVT_MAX_JITTER              (16)
/// Duration of IFS (150 us)
#define LLD_EVT_IFS_DURATION            (150)
/// Synchronization Word Duration + preamble 1MB 8us(preamble)+32us(AA)
#define LLD_EVT_SYNCWORD_DURATION_1MBPS (40)
/// Synchronization Word Duration + preamble 2MB 8us(preamble)+16us(AA)
#define LLD_EVT_SYNCWORD_DURATION_2MBPS (24)

/// CONNECT_REQ PDU duration
#define LLD_EVT_CONNECT_REQ_DURATION    (312)

/// Default RX window offset 1MBPS
#define LLD_EVT_RX_WIN_DFT_OFF_1MBPS   (LLD_EVT_SYNCWORD_DURATION_1MBPS - (LLD_EVT_DEFAULT_RX_WIN_SIZE / 2))

/// Default RX window offset 2MBPS
#define LLD_EVT_RX_WIN_DFT_OFF_2MBPS   (LLD_EVT_SYNCWORD_DURATION_2MBPS - (LLD_EVT_DEFAULT_RX_WIN_SIZE / 2))

/// Path delay (in us) compensation for RX window @1MBPS
#define LLD_EVT_RX_WIN_PATHDLY_COMP_1MBPS   (0)

/// Path delay (in us) compensation for RX window @2MBPS
#define LLD_EVT_RX_WIN_PATHDLY_COMP_2MBPS   (6)

/// Maximum duration of a sleep, in low power clock cycles (around 300s)
#define LLD_EVT_MAX_SLEEP_DURATION      12000000

/// Default sleep duration when no event is programmed (in slot count)
#define LLD_EVT_DEFAULT_SLEEP_DURATION  8000

/// Maximum slave latency supported when total SCA is 1000ppm
#define LLD_EVT_MAX_LATENCY             450

/*
 * Setting is calculated arbitrary on the bellow assumtions (for worst case)
 *  CS Size is 50 (16 bits access and DM field included)
 *   HW CS Read is 44 access
 *   HW CS Update is 18 access
 *  Tx Desc Size is 5 (16 bits access)
 *   HW TX Desc Read is 5 access
 *   HW Tx Desc Update is 1 access
 *  Rx Desc Size is 7
 *   HW Rx Desc Read is 3 access
 *   HW Rx Desc Update is 5 access
 *  Worst case for aborting current frame is 76 access
 *
 *  Assuming IP is running at 8MHz = 0.125 us
 *  Assuming EM waitstate is 4 for each access
 *  Assuming Hopping is 10 us
 *  Assuming Radio busy is 26 us (Ripple only)
 *  Assuming Highest Radio pwrup is 110 us
 *
 *  Total time needed for abort with no bitoffset (worst case) is = 76*0.125*4 + 10 + 26 + 110 = 184 us
 *
 *  PREFETCH ABORT TIME is = 624 - 184 = 440
 *  PREFETCH TIME is       = 184 + 26 us of Margin for abort and skip decision = 210
 */

/// Prefetch Abort time (in us)
#define LLD_EVT_ABORT_CNT_DURATION_US  (440)

/// Prefetch time (in us)
#define LLD_EVT_PREFETCH_TIME_US       (210)

/// Define the margin needed to reschedule correctly a scan/adv in case where the interval == window
/// or if the interval is < LLD_EVT_START_MARGIN
#define LLD_EVT_START_MARGIN         (2)

/*****************************************************************************************
 * Event Flags (Status)
 ****************************************************************************************/

enum lld_evt_flag
{
    /// Flag forcing the slave to wait for the next acknowledgment
    LLD_EVT_FLAG_WAITING_ACK              = (1 << 0),
    /// Flag that indicate we are waiting for first sync
    LLD_EVT_FLAG_WAITING_SYNC             = (1 << 1),
    /// Inform that a LLCP will be sent, used to update priority
    LLD_EVT_FLAG_WAITING_TXPROG           = (1 << 2),
    /// Flag forcing the slave to wake up at instant
    LLD_EVT_FLAG_WAITING_INSTANT          = (1 << 3),
    /// Delete the event after next End of Event ISR
    LLD_EVT_FLAG_WAITING_EOEVT_TO_DELETE  = (1 << 4),
    /// Do not restart the element
    LLD_EVT_FLAG_NO_RESTART               = (1 << 5),
    /// Indicate that end of event is due to an APFM interrupt
    LLD_EVT_FLAG_APFM                     = (1 << 6),
    /// Mark that latency is active for the link
    LLD_EVT_FLAG_LATENCY_ACTIVE           = (1 << 7),
};


/*
 * MACROS
 ****************************************************************************************
 */

/// Set Event status flag
#define LLD_EVT_FLAG_SET(evt, flag)          \
                (evt->evt_flag |= (LLD_EVT_FLAG_ ## flag))
/// Reset Event status flag
#define LLD_EVT_FLAG_RESET(evt, flag)        \
                (evt->evt_flag &= (~(LLD_EVT_FLAG_ ## flag)))
/// Get Event status flag
#define LLD_EVT_FLAG_GET(evt, flag)          \
                (evt->evt_flag & (LLD_EVT_FLAG_ ## flag))

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum lld_evt_mode
{
    LLD_EVT_ADV_MODE             = 0,
    LLD_EVT_SCAN_MODE,
    LLD_EVT_TEST_MODE,
    LLD_EVT_MST_MODE,
    LLD_EVT_SLV_MODE,

    LLD_EVT_MODE_MAX
};



/// Type of events - Format value set in the Control Structure
enum lld_evt_cs_format
{
    // Master Connect
    LLD_MASTER_CONNECTED     = 0x02,
    // Slave Connect
    LLD_SLAVE_CONNECTED      = 0x03,
    // Low Duty Cycle Advertiser
    LLD_LD_ADVERTISER        = 0x04,
    // High Duty Cycle Advertiser
    LLD_HD_ADVERTISER        = 0x05,
    // Passive Scanner
    LLD_PASSIVE_SCANNING     = 0x08,
    // Active Scanner
    LLD_ACTIVE_SCANNING      = 0x09,
    // Initiator
    LLD_INITIATING           = 0x0F,
    // Tx Test Mode
    LLD_TXTEST_MODE          = 0x1C,
    // Rx Test Mode
    LLD_RXTEST_MODE          = 0x1D,
    // Tx / Rx Test Mode
    LLD_TXRXTEST_MODE        = 0x1E,
};


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Synchronization counters
struct lld_evt_anchor
{
    /// Base time counter value of the latest found sync
    uint32_t basetime_cnt;
    /// Fine time counter value of the latest found sync
    uint16_t finetime_cnt;
    /// Event counter of the of the latest found sync
    uint16_t evt_cnt;
};

/// Non connected event information
struct lld_non_conn
{
    /// Scan Window Size
    uint32_t window;
    /// Anchor timestamp
    uint32_t anchor;
    /// Event end time stamp
    uint32_t end_ts;
    /// use to know if it's an initiate procedure
    bool     initiate;
    /// use to know if connect request has been sent
    bool     connect_req_sent;
};

/// Connected event information
struct lld_conn
{
    /// Synchronization Window Size (in us)
    uint32_t sync_win_size;
    /// SCA Drift (in us)
    uint32_t sca_drift;
    /// Instant of the next action (in events)
    uint16_t instant;
    /// Latency
    uint16_t latency;
    /// Event counter
    uint16_t counter;
    /// Number of connection events missed since last anchor point
    uint16_t missed_cnt;
    /// Minimum duration of the event or frame (in slots)
    uint16_t duration_dft;
    /// Update offset
    uint16_t update_offset;
    /// Effective maximum tx time
    uint16_t eff_max_tx_time;
    /// Effective maximum tx size
    uint16_t eff_max_tx_size;
    /// Update window size
    uint8_t  update_size;
    /// Describe the action to be done when instant occurs
    uint8_t  instant_action;
    /// Master sleep clock accuracy
    uint8_t  mst_sca;
    /// value of the latest More Data bit received
    uint8_t  last_md_rx;
    /// TX programmed packet counter
    uint8_t  tx_prog_pkt_cnt;
    /// Default RX windows offset
    uint8_t  rx_win_off_dft;
    /// Default RX windows path delay compensation
    uint8_t  rx_win_pathdly_comp;
    #if (BLE_2MBPS)
    /// TX phy to be programmed
    uint8_t  tx_phy;
    /// RX phy to be programmed
    uint8_t  rx_phy;
    #endif // (BLE_2MBPS)
    /// Wait for a sync due to a connection update
    bool wait_con_up_sync;
	
	uint32_t win_size_backup;  //us
};

/// Structure describing an event
struct lld_evt_tag
{
    /// Information about the latest found synchronization
    struct lld_evt_anchor anchor_point;

    /// List of TX Data descriptors ready for transmission (i.e. not yet chained with the CS)
    struct common_list tx_acl_rdy;
    /// List of TX Data descriptors ready to be freed (i.e. already chained in the CS)
    struct common_list tx_acl_tofree;
    #if (BLE_CENTRAL || BLE_PERIPHERAL)
    /// List of TX LLCP descriptors ready for transmission (i.e. not yet chained with the CS)
    struct common_list tx_llcp_pdu_rdy;
    #endif // (BLE_CENTRAL || BLE_PERIPHERAL)
    /// List of TX LLCP descriptors programmed for transmission (i.e. chained with the CS)
    struct common_list tx_prog;


    /// Interval element pointer linked to this event
    struct ea_interval_tag* interval_elt;

    /// event information for connected and non-connected activity
    union lld_evt_info
    {
        /// Non connected event information
        struct lld_non_conn non_conn;

        /// Connected event information
        struct lld_conn conn;
    } evt;

    /// Connection Handle
    uint16_t conhdl;
    /// Control structure pointer address
    uint16_t cs_ptr;
    /// Connection Interval
    uint16_t interval;
    /// Number of RX Descriptors already handled in the event
    uint8_t rx_cnt;
    /// Mode of the link (Master connect, slave connect, ...)
    uint8_t mode;
    /// TX Power
    uint8_t tx_pwr;
    /// Default priority
    uint8_t default_prio;

    /// Internal status
    uint8_t evt_flag;

    /// Flag delete on going
    bool delete_ongoing;
};

/// Structure describing the parameters for a connection update
struct lld_evt_update_tag
{
    /// Offset to be put in the connection update request
    uint16_t win_offset;
    /// Instant for the connection update
    uint16_t instant;
    /// Size of the window to be put in the connection update request
    uint8_t win_size;
};

/// Structure describing an event
struct lld_evt_int_tag
{
    /// List element for chaining
    struct common_list_hdr hdr;
    /// List of events attached to this interval
    struct lld_evt_tag *evt;
    /// Number of free slots
    uint32_t freeslot;
    /// Base interval time
    uint16_t int_base;
};

/// Environment structure for the LLD module
struct lld_evt_env_tag
{
    /// List of programmed element
    struct common_list elt_prog;
    /// List of programmed element
    struct common_list elt_wait;
    /// List of element pending to be freed
    struct common_list elt_deferred;
    /// List of rx packet to handle
    struct common_list rx_pkt_deferred;
    /// List of element pending to be deleted
    struct common_list elt_to_be_deleted;
    /// Accuracy of the low power clock connected to the BLE core
    uint8_t sca;
    /// Used to know if resolvable private address shall be renewed
    bool renew;
    /// Compensation in us of the HW behavior when fine_cnt compensation equals zero, HW does not execute 1 us.
    uint8_t hw_wa_sleep_compensation;
};

/// Environment structure for the LLD module
struct lld_evt_wait_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct common_list_hdr hdr;
    /// Pointer on the waiting element
    struct ea_elt_tag *elt_ptr;
};

/// Deferred element structure
struct lld_evt_deferred_tag
{
    /// List element for chaining in the Even Arbiter lists
    struct common_list_hdr hdr;

    /// Pointer on the deferred element
    struct ea_elt_tag *elt_ptr;

    /// Type of deferring
    uint8_t type;

    /// Number of used RX Descriptors
    uint8_t rx_desc_cnt;
};

/// Delete element structure
struct lld_evt_delete_tag
{
    /// List element for chaining in the delete lists
    struct common_list_hdr hdr;

    /// Pointer on the deferred element
    struct ea_elt_tag *elt_ptr;

    /// Tparameters of teh deletion
    bool flush;
    bool send_ind;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/// Environment of the LLDEVT module
extern struct lld_evt_env_tag lld_evt_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Get sleep clock accuracy
 *
 * @return The sleep clock accuracy as defined in the standard
 *
 ****************************************************************************************
 */
__INLINE uint8_t lld_evt_sca_get(void)
{
    // Read sleep clock accuracy from the environment
    return (lld_evt_env.sca);
}

/**
 ****************************************************************************************
 * @brief Get current time value from HW
 *
 * @return The current time in units of 625us
 *
 ****************************************************************************************
 */
__INLINE uint32_t lld_evt_time_get(void)
{
    // Sample the base time count
    ble_samp_setf(1);
    while(ble_samp_getf());
    // Read current time in HW
    return (ble_basetimecnt_get());
}


__INLINE void lld_evt_time_get_us(uint32_t *slot ,  uint32_t *slot1)
{
    // Sample the base time count
    ble_samp_setf(1);
    while(ble_samp_getf());

	*slot = ble_basetimecnt_get();
	*slot1 = ble_finetimecnt_get();

    // Read current time in HW    
    //return (ble_finetimecnt_get());
}


/**
 ****************************************************************************************
 * @brief Compare absolute times
 *
 * The absolute time difference between time1 and time2 is supposed to be less than the
 * maximum interval time
 *
 * @param[in] time1 First time to compare
 * @param[in] time2 Second time to compare
 *
 * @return true if time1 is smaller than time2.
 ****************************************************************************************
 */
__INLINE bool lld_evt_time_cmp(uint32_t time1,
                               uint32_t time2)
{
    return (((time1 - time2) & BLE_BASETIMECNT_MASK) > MAX_INTERVAL_TIME);
}

/**
 ****************************************************************************************
 * @brief Check if time passed as parameter is in the past
 *
 * @param[in] time Time to be compare with current time to see if it in the past
 *
 * @return true if time is in the past, false otherwise
 *
 ****************************************************************************************
 */
__INLINE bool lld_evt_time_past(uint32_t time)
{
    // Compare time and current time
    return(lld_evt_time_cmp(time & BLE_BASETIMECNT_MASK, lld_evt_time_get()));
}

/**
 ****************************************************************************************
 * @brief Computes the maximum drift according to the master clock accuracy and the delay
 * passed as parameters
 *
 * @param[in] delay       Duration for which the drift is computed
 * @param[in] master_sca  Sleep clock accuracy of the master
 *
 * @return The value of the RX window size formatted for the RXWINCNTL field of the
 * control structure
 *
 ****************************************************************************************
 */
uint16_t lld_evt_drift_compute(uint16_t delay, uint8_t master_sca);

/**
 ****************************************************************************************
 * @brief Create a connection or scanning event and chain it in the scheduling and
 * interval lists
 *
 * @param[in] handle      Connection handle for which the event is created (LLD_ADV_HDL if
 *                        non-connected event)
 * @param[in] duration    Expected duration of the event, in units of 625us. This duration
 *                        can be reduced if scheduling of other events is requiring that.
 * @param[in] mininterval Minimum interval for the connection event
 * @param[in] maxinterval Maximum interval for the connection event
 * @param[in] latency     Requested latency for the event
 *
 * @return The pointer to the event just created
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_evt_scan_create(uint16_t handle, uint16_t latency);

/**
 ****************************************************************************************
 * @brief Create an advertising event and chain it in the scheduling list
 *
 * @param[in] handle      Connection handle for which the event is created (LLD_ADV_HDL if
 *                        non-connected event)
 * @param[in] mininterval Minimum interval for the advertising event
 * @param[in] maxinterval Maximum interval for the advertising event
 * @param[in] restart_pol Requested restart policy, i.e. LLD_ADV_RESTART or LLD_NO_RESTART
 *
 * @return The pointer to the event just created
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_evt_adv_create(uint16_t handle,
                                      uint16_t mininterval,
                                      uint16_t maxinterval,
                                      bool restart_pol);

#if (BLE_CENTRAL || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief Create a connection event for parameter update
 *
 * @param[in]  evt_old     Pointer to the current connection event
 * @param[in]  ce_len      Requested size of the connection event (in 625us slots)
 * @param[in]  mininterval Minimum interval for the connection event
 * @param[in]  maxinterval Maximum interval for the connection event
 * @param[in]  latency     Requested slave latency
 * @param[in]  pref_period preferred periodicity
 * @param[out] upd_par     Computed updated parameters, to be put in the LLCP frame
 *
 * @return The pointer to the new event created (used after instant)
 *
 ****************************************************************************************
 */
struct ea_elt_tag *lld_evt_update_create(struct ea_elt_tag *evt_old,
                                         uint16_t ce_len,
                                         uint16_t mininterval,
                                         uint16_t maxinterval,
                                         uint16_t latency,
                                         uint8_t  pref_period,
                                         struct lld_evt_update_tag *upd_par);

/**
 ****************************************************************************************
 * @brief Create a slave connection event
 *
 * @param[in]  con_par     Pointer to the decoded connection parameters
 * @param[in]  con_req_pdu Pointer to the connection request frame as received
 * @param[in]  evt_adv     Pointer to the advertising event that triggered the connection
 * @param[in]  conhdl      Connection handle to the established connection
 *
 * @return The pointer to the slave event created
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_evt_move_to_slave(struct llc_create_con_req_ind *con_par,
                                         struct llm_pdu_con_req_rx *con_req_pdu,
                                         struct ea_elt_tag *elt_adv,
                                         uint16_t conhdl);
/**
 ****************************************************************************************
 * @brief Indicates to the LLD the occurrence of a connection parameter update.
 *
 * @param[in]  param_pdu   Pointer to the connection parameter update PDU
 * @param[in]  evt_old     Pointer to the current event used for this connection
 *
 ****************************************************************************************
 */
void lld_evt_slave_update(struct llcp_con_upd_ind const *param_pdu,
                          struct ea_elt_tag *elt_old);

/**
 ****************************************************************************************
 * @brief Indicates to the LLD to move from initiating to master connected state.
 *
 * @param[in]  evt      Pointer to the event used for initiation
 * @param[in]  conhdl   Handle of the new master connection
 * @param[in]  rx_hdl   Handle of the rx descriptor where the ADV has been detected
 *
 ****************************************************************************************
 */
struct ea_elt_tag* lld_evt_move_to_master(struct ea_elt_tag *elt_scan, uint16_t conhdl, struct llc_create_con_req_ind const *pdu_tx, uint8_t rx_hdl);
#endif //(BLE_CENTRAL || BLE_PERIPHERAL)

/**
 ****************************************************************************************
 * @brief Program the next occurrence of the slave event passed as parameter
 * In case the slave event passed as parameter is far enough in the future (e.g. due to
 * slave latency), the event is canceled and replaced with the earliest possible one. This
 * function is called when a data has been pushed for transmission in order to send it as
 * soon as possible even if slave latency is used.
 *
 * @param[in] elt        Element to be deleted
 ****************************************************************************************
 */
void lld_evt_schedule_next(struct ea_elt_tag *elt);


/**
 ****************************************************************************************
 * @brief Initialization of the BLE event scheduler
 *
 * This function initializes the lists used for event scheduling.
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 ****************************************************************************************
 */
void lld_evt_init(bool reset);

/**
 ****************************************************************************************
 * @brief Initialization of BLE event environement
 ****************************************************************************************
 */
void lld_evt_init_evt(struct lld_evt_tag *evt);

/**
 ****************************************************************************************
 * @brief Handle insertion of an element in Event Arbitrer list of elements
 ****************************************************************************************
 */
void lld_evt_elt_insert(struct ea_elt_tag *elt, bool inc_prio);

/**
 ****************************************************************************************
 * @brief Handle removing of an element in Event Arbitrer list of elements
 ****************************************************************************************
 */
bool lld_evt_elt_delete(struct ea_elt_tag *elt, bool flush_data, bool send_indication);

/**
 ****************************************************************************************
 * @brief Kernel event scheduled when BLE events has to be programmed in the HW
 *
 * This function is a deferred action of the BLE wake up interrupt. It programs the data
 * Tx/Rx exchange in the control structure corresponding to the event, and programs the
 * target time of the next event to be scheduled.
 *
 ****************************************************************************************
 */
void lld_evt_schedule(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Kernel event scheduled when a BLE event has to be handled by the HW
 *
 * This function is a deferred action of the BLE end of event interrupt. It flushes the
 * data Tx/Rx exchange corresponding to the event, and programs the
 * target time of the next event to be scheduled.
 *
 ****************************************************************************************
 */
void lld_evt_end(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Kernel event scheduled when a BLE RX has to be handled by the HW
 *
 * This function is a deferred action of the BLE RX interrupt. It flushes the
 * data Tx/Rx exchange corresponding to the event.
 *
 ****************************************************************************************
 */
void lld_evt_rx(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief End of sleep interrupt handler
 *
 * This function is called under interrupt when an end of sleep interrupt is generated by
 * the BLE core. It sets the associated kernel event in order to perform the handling as a
 * deferred action in background context.
 *
 ****************************************************************************************
 */
void lld_evt_start_isr(void);


/**
 ****************************************************************************************
 * @brief End of event/frame interrupt handler
 *
 * This function is called under interrupt when an end of event/frame interrupt is
 * generated by the BLE/BT core.
 *
 * param[in] apfm  - Indicate if end of event is due to an apfm interrupt
 *
 ****************************************************************************************
 */
void lld_evt_end_isr(bool apfm);

/**
 ****************************************************************************************
 * @brief RX interrupt handler
 *
 * This function is called under interrupt when an RX interrupt is generated by
 * the BLE core. It sets the associated kernel event in order to perform the handling as a
 * deferred action in background context.
 *
 ****************************************************************************************
 */
void lld_evt_rx_isr(void);

/**
 ****************************************************************************************
 * @brief General purpose timer interrupt handler
 *
 * This function is called under interrupt when a general purpose timer interrupt is
 * generated by the BLE core. It sets the associated kernel event in order to perform the
 * handling as a deferred action in background context.
 *
 ****************************************************************************************
 */
void lld_evt_timer_isr(void);


/**
 ****************************************************************************************
 * @brief Check if event has an alternative event, if yes, delete it.
 *
 * @param[in] evt          Pointer to the event which can have alternative event
 ****************************************************************************************
 */
void lld_delete_alt_evt(struct lld_evt_tag * evt);

/**
 ****************************************************************************************
 * @brief Computes the next unmapped channel index according to the hopping mechanism.
 * The previous channel index is taken from the control structure, and once the next
 * unmapped channel is computed, it is put back in the control structure.
 *
 * @param[in] evt The event for which the next channel has to be computed
 *
 ****************************************************************************************
 */
void lld_evt_channel_next(uint16_t conhdl, int16_t nb_inc);

/**
 ****************************************************************************************
 * @brief Try to cancel an event fur to host or EA request
 *
 * @param[in] elt           Pointer to the element to be canceled
 *
 ****************************************************************************************
 */
void lld_evt_canceled(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Function which handles the non IRQ functionalities: RX data, ...
 *
 ****************************************************************************************
 */
void lld_evt_deffered_elt_handler(void);

/**
 ****************************************************************************************
 * @brief Dummy call back function to match with EA API
 *
 * @param[in] elt           Pointer to the element to be stopped
 *
 ****************************************************************************************
 */
void lld_evt_prevent_stop(struct ea_elt_tag *elt);

/**
 ****************************************************************************************
 * @brief Try to restart the event by updating scheduling parameters
 *
 * @param[in] elt           Pointer to the element to be restarted
 * @param[in] rejected      Flag to indicate if the restart comes from a reject
 *
 * @return True if element can be restarted, False else
 ****************************************************************************************
 */
bool lld_evt_restart(struct ea_elt_tag *elt, bool rejected);

/**
 ****************************************************************************************
 * @brief Try push a delete element in the list
 *
 * @param[in] elt           Pointer to the element to be deleted
 * @param[in] flush         Flag to indicate if the tx data should be flushed
 * @param[in] flush         Flag to indicate if an event should be sent to the host
 *
 ****************************************************************************************
 */
void lld_evt_delete_elt_push(struct ea_elt_tag *elt, bool flush, bool send_indication);

/// @} LLDEVT

#endif // LLD_EVT_H_
