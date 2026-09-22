/*
 * Host contract for A003 R14's PS2 projection.
 */

#include "config/rfb_runtime_profile.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    pstvnc_transport_session_config_t config;
    pstvnc_transport_session_config_t sentinel;
    pstvnc_transport_session_config_t before;

    assert(pstvnc_config_rfb_runtime_profile_selected_mode() ==
           PSTVNC_CONFIG_RFB_ON);

    memset(&config, 0, sizeof(config));
    assert(pstvnc_config_rfb_runtime_profile_selected(&config));
    assert(config.rfb_queue_capacity == 32768u);
    assert(config.rfb_initial_credit_bytes == 32768u);
    assert(config.rfb_credit_batch_bytes == 8192u);
    assert(config.rfb_credit_flush_on_empty == 1);
    assert(config.rfb_credit_return_enabled == 1);
    assert(config.receiver_thread_priority == 63);
    assert(config.receiver_thread_stack_size == 16384u);
    assert(config.max_data_payload == 8192u);

    memset(&sentinel, 0xA5, sizeof(sentinel));
    before = sentinel;
    assert(!pstvnc_config_rfb_runtime_profile_project(
        PSTVNC_CONFIG_RFB_OFF, &sentinel));
    assert(memcmp(&sentinel, &before, sizeof(sentinel)) == 0);

    assert(!pstvnc_config_rfb_runtime_profile_project(
        (pstvnc_config_rfb_mode_t)99, &sentinel));
    assert(memcmp(&sentinel, &before, sizeof(sentinel)) == 0);
    return 0;
}
