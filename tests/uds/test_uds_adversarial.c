/*
 * SPDX-License-Identifier: LicenseRef-STM32-CANopen-Research-Education-Commercial-1.0
 */
#include "isotp.h"
#include "uds.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static IsoTpCanFrame frame(uint32_t id, uint8_t dlc, const uint8_t *data) {
    IsoTpCanFrame result = {0};
    result.can_id = id;
    result.dlc = dlc;
    if ((data != NULL) && (dlc <= ISOTP_MAX_FRAME_DATA)) {
        (void)memcpy(result.data, data, dlc);
    }
    return result;
}

static void test_isotp_rx_adversarial(void) {
    IsoTpConfig config = {1U, 0U, 10U, 10U};
    IsoTpRx rx;
    IsoTpRxEvent event;
    const uint8_t ff_invalid_length[] = {0x10U, 0x07U, 0U, 0U, 0U, 0U, 0U, 0U};
    const uint8_t ff_valid[] = {0x10U, 0x14U, 0U, 1U, 2U, 3U, 4U, 5U};
    const uint8_t cf_wrong_sequence[] = {0x22U, 6U, 7U, 8U, 9U, 10U, 11U, 12U};
    IsoTpCanFrame input = {0};
    isotp_rx_init(&rx, &config, 0x7E0U, 0x7E8U);

    input = frame(0x7E0U, 0U, NULL);
    assert(isotp_rx_feed(&rx, &input, 0U, &event) == ISOTP_ERR_FORMAT);
    input = frame(0x7E0U, 8U, ff_invalid_length);
    assert(isotp_rx_feed(&rx, &input, 0U, &event) == ISOTP_ERR_FORMAT);
    input = frame(0x7E0U, 8U, ff_valid);
    assert(isotp_rx_feed(&rx, &input, 0U, &event) == ISOTP_NEED_FLOW_CONTROL);
    assert(event.has_flow_control && event.flow_control.can_id == 0x7E8U);
    input = frame(0x7E0U, 8U, cf_wrong_sequence);
    assert(isotp_rx_feed(&rx, &input, 1U, &event) == ISOTP_ERR_SEQUENCE);
    input = frame(0x7E0U, 8U, ff_valid);
    assert(isotp_rx_feed(&rx, &input, 0U, &event) == ISOTP_NEED_FLOW_CONTROL);
    assert(isotp_rx_tick(&rx, 11U) == ISOTP_ERR_TIMEOUT);
    input = frame(0x701U, 1U, (const uint8_t[]){0U});
    assert(isotp_rx_feed(&rx, &input, 0U, &event) == ISOTP_OK);
}

static void test_isotp_tx_adversarial(void) {
    IsoTpConfig config = {1U, 0U, 10U, 10U};
    IsoTpTx tx;
    IsoTpCanFrame output;
    const uint8_t payload[8] = {0U};
    const uint8_t bad_stmin[] = {0x30U, 0U, 0x80U};
    const uint8_t overflow_fc[] = {0x32U, 0U, 0U};
    IsoTpCanFrame flow_control = {0};
    isotp_tx_init(&tx, &config, 0x7E0U, 0x7E8U);
    assert(isotp_tx_start(&tx, payload, sizeof(payload), 0U, &output) == ISOTP_TX_FRAME_READY);
    flow_control = frame(0x7E0U, 3U, bad_stmin);
    assert(isotp_tx_feed_flow_control(&tx, &flow_control, 0U) == ISOTP_ERR_FLOW_CONTROL);
    assert(isotp_tx_start(&tx, payload, sizeof(payload), 0U, &output) == ISOTP_TX_FRAME_READY);
    flow_control = frame(0x7E0U, 3U, overflow_fc);
    assert(isotp_tx_feed_flow_control(&tx, &flow_control, 0U) == ISOTP_ERR_FLOW_CONTROL);
    assert(isotp_tx_start(&tx, payload, ISOTP_MAX_PAYLOAD + 1U, 0U, &output)
           == ISOTP_ERR_OVERFLOW);
    assert(isotp_tx_start(&tx, payload, sizeof(payload), 0U, &output) == ISOTP_TX_FRAME_READY);
    assert(isotp_tx_tick(&tx, 11U) == ISOTP_ERR_TIMEOUT);
}

static void test_uds_bounds_and_negative_responses(void) {
    UdsServer server;
    UdsCallbacks callbacks = {0};
    uint8_t request_short[] = {0x22U};
    uint8_t request_unknown[] = {0x99U};
    uint8_t request_tester[] = {0x3EU, 0U};
    uint8_t response[8] = {0xA5U};
    uint16_t response_len = 0U;
    uds_server_init(&server, &callbacks, NULL, 0U);
    assert(uds_server_handle(&server, request_short, sizeof(request_short), response,
                             &response_len, sizeof(response), 0U)
           == UDS_RESULT_OK);
    assert(response_len == 3U && response[0] == 0x7FU && response[1] == 0x22U &&
           response[2] == UDS_NRC_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT);
    response_len = 0U;
    assert(uds_server_handle(&server, request_unknown, sizeof(request_unknown), response,
                             &response_len, sizeof(response), 0U)
           == UDS_RESULT_OK);
    assert(response_len == 3U && response[0] == 0x7FU && response[1] == 0x99U &&
           response[2] == UDS_NRC_SERVICE_NOT_SUPPORTED);
    response_len = 0U;
    assert(uds_server_handle(&server, request_tester, sizeof(request_tester), response,
                             &response_len, 0U, 0U) != UDS_RESULT_OK);
    assert(response_len == 0U);
}

int main(void) {
    test_isotp_rx_adversarial();
    test_isotp_tx_adversarial();
    test_uds_bounds_and_negative_responses();
    return 0;
}
