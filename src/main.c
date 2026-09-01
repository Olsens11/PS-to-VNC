#include "app.h"
#include "platform/ps2_system.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    (void)pstvnc_app_run();
    pstvnc_ps2_system_exit_to_menu();
    return 0;
}
