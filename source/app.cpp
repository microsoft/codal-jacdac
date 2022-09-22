#include "CodalJacdac.h"

using namespace codal;

extern "C" {

void init_jacscript_manager(void);
void init_local_services(void);

void app_init_services() {
    jd_role_manager_init();
    init_jacscript_manager();
    init_local_services();

    // tsagg_init(&noop_cloud);
}
}