/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Optional STM32 classic-CAN transport facade.
 *
 * This source is intentionally not linked into the default CANopenNode target.
 * CANopenNode_STM32 already owns HAL CAN callbacks and the controller lifecycle
 * for the production node. Use this facade for standalone diagnostics, a test
 * client, or a future adapter after assigning exclusive callback ownership.
 */
#include "can_port.h"

#ifdef CAN_PORT_STM32

#include <errno.h>
#include <stddef.h>

static CAN_HandleTypeDef *s_hcan;
/*
 * s_rx_callback may be set or cleared from non-ISR context. The dispatcher may
 * be invoked from an ISR. To avoid torn pointer reads and races between the
 * contexts, access the callback pointer with atomic operations.
 *
 * Note: this uses GCC/Clang __atomic builtins for minimal toolchain impact.
 * Prefer a dedicated ISR->mainline ring buffer for production systems so that
 * ISR work is bounded and application callbacks always run outside interrupt
 * context.
 */
static can_port_rx_callback_t s_rx_callback = NULL;

int
can_port_stm32_bind(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL) {
        return -EINVAL;
    }
    s_hcan = hcan;
    return 0;
}

int
can_port_init(uint32_t bitrate) {
    (void)bitrate;
    if (s_hcan == NULL) {
        return -ENODEV;
    }

    /* Bit timing is CubeMX/board-owned. The caller must set it before binding. */
    if (HAL_CAN_Start(s_hcan) != HAL_OK) {
        return -EIO;
    }
    if (HAL_CAN_ActivateNotification(s_hcan,
                                     CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING
                                         | CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_ERROR)
        != HAL_OK) {
        (void)HAL_CAN_Stop(s_hcan);
        return -EIO;
    }
    return 0;
}

int
can_port_send(uint32_t id, uint8_t *data, uint8_t len) {
    CAN_TxHeaderTypeDef header = {0};
    uint32_t mailbox = 0U;

    if (s_hcan == NULL || data == NULL || len > CAN_PORT_MAX_DLC || id > 0x7FFU) {
        return -EINVAL;
    }
    if (HAL_CAN_GetTxMailboxesFreeLevel(s_hcan) == 0U) {
        return -EAGAIN;
    }

    header.StdId = id;
    header.ExtId = 0U;
    header.IDE = CAN_ID_STD;
    header.RTR = CAN_RTR_DATA;
    header.DLC = len;
    header.TransmitGlobalTime = DISABLE;
    return HAL_CAN_AddTxMessage(s_hcan, &header, data, &mailbox) == HAL_OK ? 0 : -EIO;
}

void
can_port_register_rx(can_port_rx_callback_t cb) {
    /* Store the callback pointer atomically so ISR can safely read it. */
    __atomic_store_n(&s_rx_callback, cb, __ATOMIC_RELEASE);
}

int
can_port_poll(uint32_t timeout_ms) {
    (void)timeout_ms;
    /* STM32 reception is interrupt-driven; callbacks dispatch frames. */
    return 0;
}

void
can_port_deinit(void) {
    if (s_hcan != NULL) {
        (void)HAL_CAN_DeactivateNotification(s_hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING
                                                          | CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_ERROR);
        (void)HAL_CAN_Stop(s_hcan);
    }
    __atomic_store_n(&s_rx_callback, (can_port_rx_callback_t)NULL, __ATOMIC_RELEASE);
    s_hcan = NULL;
}

void
can_port_stm32_dispatch_rx_from_isr(uint32_t id, uint8_t *data, uint8_t len) {
    /* Load the callback atomically to avoid torn reads while registration
     * changes concurrently. Use acquire semantics to pair with release store. */
    can_port_rx_callback_t cb = __atomic_load_n(&s_rx_callback, __ATOMIC_ACQUIRE);
    if (cb != NULL && data != NULL && len <= CAN_PORT_MAX_DLC && id <= 0x7FFU) {
        cb(id, data, len);
    }
}

#else

/* A deliberate build failure prevents accidental selection of this source for
 * a host transport. Compile vcan_port.c for SocketCAN builds instead. */
#error "can_port.c requires CAN_PORT_STM32; use vcan_port.c for host builds"

#endif
