#include "app.h"
#include "platform/ps2_system.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /*
     * main owns no product mechanisms. The coordinator runs the complete
     * application lifecycle, and every return—success is not currently a live
     * outcome—converges on the known PS2 system-menu destination.
     */
    (void)pstvnc_app_run();
    pstvnc_ps2_system_exit_to_menu();
    return 0;
}
