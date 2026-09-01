#include "app.h"

#include "platform/ps2_network.h"
#include "platform/ps2_system.h"
#include "rfb.h"

#define PSTVNC_RFB_PORT 5900u

/*
 * M1A proves transport/handshake only. The current historical Pi desktop is
 * 1280x720, so this explicit temporary ServerInit envelope lets the clean
 * client prove the private-link/RFB foundation before M1B owns a fixed 480p
 * framebuffer and desktop-size contract.
 */
#define PSTVNC_M1A_MAX_DESKTOP_WIDTH  1280u
#define PSTVNC_M1A_MAX_DESKTOP_HEIGHT 720u

static void pstvnc_app_exit_after_message(void)
{
    pstvnc_ps2_delay_ms(5000);
    pstvnc_ps2_exit_to_system_menu();
}

int pstvnc_app_run(void)
{
    pstvnc_rfb_session_t rfb;

    if (pstvnc_ps2_system_bootstrap() < 0)
        return 1;

    pstvnc_ps2_console_init();
    pstvnc_ps2_console_printf("PS-to-VNC clean reconstruction M1A");
    pstvnc_ps2_console_printf("Stage: PS2 system bootstrap OK");

    pstvnc_ps2_console_printf("Stage: starting private Ethernet");

    if (pstvnc_ps2_network_start() < 0) {
        pstvnc_ps2_console_printf("FAIL: PS2 network initialization");
        pstvnc_app_exit_after_message();
        return 1;
    }

    pstvnc_ps2_console_printf(
        "PS2 %s -> Pi %s",
        PSTVNC_PS2_IPV4,
        PSTVNC_PI_IPV4);

    pstvnc_ps2_console_printf("Stage: waiting for Ethernet link");

    if (pstvnc_ps2_network_wait_for_link(10000) < 0) {
        pstvnc_ps2_console_printf("FAIL: Ethernet link timeout");
        pstvnc_app_exit_after_message();
        return 1;
    }

    pstvnc_ps2_console_printf("Stage: Ethernet link UP");
    pstvnc_ps2_console_printf("Stage: RFB 3.8 handshake");

    pstvnc_rfb_session_init(&rfb);

    if (pstvnc_rfb_connect_and_handshake(
            &rfb,
            PSTVNC_PI_IPV4,
            PSTVNC_RFB_PORT,
            PSTVNC_M1A_MAX_DESKTOP_WIDTH,
            PSTVNC_M1A_MAX_DESKTOP_HEIGHT) < 0) {

        pstvnc_ps2_console_printf("FAIL: RFB handshake");
        pstvnc_app_exit_after_message();
        return 1;
    }

    pstvnc_ps2_console_printf(
        "PASS: RFB desktop %ux%u",
        rfb.server_init.width,
        rfb.server_init.height);

    pstvnc_ps2_console_printf(
        "Desktop: %.55s",
        rfb.desktop_name);

    pstvnc_rfb_session_close(&rfb);

    pstvnc_ps2_console_printf("M1A result: PASS");
    pstvnc_ps2_console_printf("Returning to PS2 system menu...");

    pstvnc_app_exit_after_message();
    return 0;
}
