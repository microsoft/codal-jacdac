#include "CodalJacdac.h"

using namespace codal;

void init_jacscript_manager(void);

extern "C" {

const char app_fw_version[] = "v0.0.0";
const char app_dev_class_name[] = "codal-jacdac device";

uint32_t app_get_device_class(void) {
    return 0x3126744e;
}

void app_init_services() {
    jd_role_manager_init();
}
}