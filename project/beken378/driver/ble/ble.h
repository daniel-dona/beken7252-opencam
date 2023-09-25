#ifndef _BLE_H_
#define _BLE_H_

#define BLE_DEBUG

#ifdef BLE_DEBUG
#define BLE_PRT      os_printf
#define BLE_WARN     warning_prf
#define BLE_FATAL    fatal_prf
#else
#define BLE_PRT      null_prf
#define BLE_WARN     null_prf
#define BLE_FATAL    null_prf
#endif

#define REG_BLE_XVR_BASE_ADDR                        (0x0080B400)

#define BLE_XVR_SLOT_TIME                            (REG_BLE_XVR_BASE_ADDR + 0x2a * 4)
#define BLE_XVR_RF_TX_DELAY_POSI                     (24)
#define BLE_XVR_RF_TX_DELAY_MASK                     (0xFF)
#define BLE_XVR_RF_RX_DELAY_POSI                     (16)
#define BLE_XVR_RF_RX_DELAY_MASK                     (0xFF)
#define BLE_XVR_TX_SLOT_TIME_POSI                    (8)
#define BLE_XVR_TX_SLOT_TIME_MASK                    (0xFF)
#define BLE_XVR_RX_SLOT_TIME_POSI                    (0)
#define BLE_XVR_RX_SLOT_TIME_MASK                    (0xFF)

#define REG_BLE_XVR_CHANNEL_CONFIG_ADDR              (REG_BLE_XVR_BASE_ADDR + 0x24 * 4)
#define REG_BLE_XVR_CHANNEL_VALUE_POST               (0)
#define REG_BLE_XVR_CHANNEL_VALUE_MASK               (0x7F)
#define REG_BLE_XVR_AUTO_CHANNEL_POST                (17)
#define REG_BLE_XVR_AUTO_SYNCWD_POST                 (19)

#define REG_BLE_XVR_TRX_CONFIG_ADDR                  (REG_BLE_XVR_BASE_ADDR + 0x25 * 4)
#define REG_BLE_XVR_TEST_RADIO_POST                  (13)
#define REG_BLE_XVR_HOLD_ESTIMATE_POST               (9)
#define REG_BLE_XVR_PN9_RECV_POST                    (10)

#define REG_BLE_XVR_GFSK_SYNCWD_ADDR                 (REG_BLE_XVR_BASE_ADDR + 0x20 * 4)

UINT32 ble_ctrl( UINT32 cmd, void *param );
void ble_isr(void);

#endif /* _BLE_H_ */
