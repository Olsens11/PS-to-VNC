/* Host test for logical RFB receiver-credit batching and empty-queue flush. */

#include "h1_rfb_credit_policy.h"

#include <stdio.h>

static int failures;

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); failures++; \
} } while (0)

int main(void)
{
    CHECK(!pstvnc_h1_rfb_credit_should_return(0u, 8192u, 1, 1, 1));
    CHECK(!pstvnc_h1_rfb_credit_should_return(4096u, 8192u, 0, 1, 0));
    CHECK(!pstvnc_h1_rfb_credit_should_return(4096u, 8192u, 1, 1, 0));
    CHECK(pstvnc_h1_rfb_credit_should_return(4096u, 8192u, 1, 1, 1));
    CHECK(pstvnc_h1_rfb_credit_should_return(8192u, 8192u, 0, 1, 0));
    CHECK(pstvnc_h1_rfb_credit_should_return(16384u, 8192u, 0, 1, 0));
    CHECK(!pstvnc_h1_rfb_credit_should_return(16384u, 8192u, 1, 0, 1));

    if (failures != 0) {
        fprintf(stderr, "H1_RFB_CREDIT_POLICY_TEST=FAIL failures=%d\n", failures);
        return 1;
    }

    puts("H1_RFB_CREDIT_POLICY_TEST=PASS");
    return 0;
}
