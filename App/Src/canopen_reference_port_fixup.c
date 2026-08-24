/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * See canopen_reference_port_fixup.h. This file adapts the generated
 * peripheral configuration to CANopen requirements without editing any
 * CubeMX-generated function.
 */
#include "canopen_reference_port_fixup.h"

#include "main.h"

void
CanopenReferencePortFixup_Prepare(CAN_HandleTypeDef *hcan) {
    if (hcan == NULL || hcan->Instance != CAN1) {
        return;
    }

    /* Protocol-correct bxCAN timing and error behavior for 500 kbit/s on the
     * 54 MHz APB1 clock (18 tq per bit). The generated init remains untouched;
     * these fields are adjusted and the peripheral is re-initialized through
     * HAL so MspDeInit/MspInit run normally. */
    hcan->Init.AutoRetransmission = ENABLE;
    hcan->Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan->Init.TimeSeg1 = CAN_BS1_14TQ;
    hcan->Init.TimeSeg2 = CAN_BS2_3TQ;

    if (HAL_CAN_DeInit(hcan) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_CAN_Init(hcan) != HAL_OK) {
        Error_Handler();
    }

    /* The 1 ms CANopen dispatch must not preempt CAN reception (CAN IRQs are
     * at priority 0,0 in the generated MSP). The generated code leaves TIM7
     * out of the NVIC configuration, so enable it here. */
    HAL_NVIC_SetPriority(TIM7_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
}
