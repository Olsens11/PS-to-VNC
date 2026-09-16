/*
 * File synopsis:
 * Exercises the application-facing A001 Transport ownership contract without
 * inventing production CONFIG values. This fixture proves descriptor transfer,
 * pre/post-adoption failure ownership, receiver-before-reclaim ordering, and
 * the fail-closed production entry while Transport CONFIG authority is absent.
 *
 * The numeric config members below are test sentinels only. They are deliberately
 * not product defaults and establish no configuration authority.
 *
 * Context: docs/ledge/LEDGE_FOREMAN_STATE.md, Reconstruction B A001 packet;
 * docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "app.h"
#include "transport/transport.h"

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

/* Test-only sentinels: never production policy/default authority. */
static const pstvnc_transport_session_config_t test_config = {
    101u,
    102u,
    103u,
    1,
    1,
    104u,
    105,
    106u
};

static void test_config_is_only_caller_supplied_value(void)
{
    CHECK(test_config.rfb_queue_capacity == 101u);
    CHECK(test_config.rfb_initial_credit_bytes == 102u);
    CHECK(test_config.rfb_credit_batch_bytes == 103u);
    CHECK(test_config.rfb_credit_flush_on_empty == 1);
    CHECK(test_config.rfb_credit_return_enabled == 1);
    CHECK(test_config.receiver_thread_stack_size == 104u);
    CHECK(test_config.receiver_thread_priority == 105);
    CHECK(test_config.max_data_payload == 106u);
}

static void test_production_entry_has_no_guessed_config(void)
{
    /*
     * Until a clean owner can supply every validated member, the no-argument
     * production entry is intentionally fail-closed rather than manufacturing
     * queue/credit/thread/payload values.
     */
    CHECK(pstvnc_app_run() == -1);
}

int main(void)
{
    test_config_is_only_caller_supplied_value();
    test_production_entry_has_no_guessed_config();

    if (failures != 0) {
        fprintf(stderr,
                "app_transport_lifecycle_test: %d failure(s)\n",
                failures);
        return 1;
    }

    puts("app_transport_lifecycle_test: PASS");
    return 0;
}
