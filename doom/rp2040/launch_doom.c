
#include "../d_main.h"
#include "../m_argv.h"

static char* STATIC_ARGV[] = { "" };

void launch_doom(void) {

    myargc = 0;
    myargv = STATIC_ARGV;

    D_DoomMain();

}
