#include "jdc.h"
#include "ZSingleWireSerial.h"

namespace jdc {

static uint32_t dev_class = 0x3126744e;
static char *dev_class_name;
static char *fw_version;

extern "C" {
const char *app_get_fw_version(void) {
    return fw_version ? fw_version : "v0.0.0";
}

const char *app_get_dev_class_name(void) {
    return dev_class_name ? dev_class_name : "codal-jacdac device";
}

uint32_t app_get_device_class(void) {
    return dev_class;
}
}

void bridge_init();

static ZSingleWireSerial *sws;

//%
void setParameters(uint32_t cls, String ver, String name) {
    dev_class = cls;
    jd_free(dev_class_name);
    dev_class_name = jd_strdup(name->getUTF8Data());
    jd_free(fw_version);
    fw_version = jd_strdup(ver->getUTF8Data());
}

//%
void start() {
    if (sws)
        return;

    // allow disabling SWS with the following anywhere in the TS code:
    // namespace userconfig { export const PIN_JACK_TX = 0xdead }
    if (getConfig(CFG_PIN_JACK_TX, 0) == 0xdead) {
        DMESG("Jacdac SWS disabled");
        return;
    }

#ifdef MICROBIT_CODAL
    sws = new ZSingleWireSerial(uBit.io.P12);
#else
    sws = new ZSingleWireSerial(*LOOKUP_PIN(JACK_TX));
#endif

    jdhw_init(sws);
    bridge_init();
}

//%
int deploy(Buffer jacsprog) {
    return jacscriptmgr_deploy(jacsprog->data, jacsprog->length);
}

//%
int numServiceInstances(int serviceClass) {
    int cnt = 0;
    for (jd_device_t *p = jd_devices; p; p = p->next) {
        if (serviceClass == 0)
            cnt++;
        else
            for (int i = 1; i < p->num_services; ++i) {
                if (p->services[i].service_class == (unsigned)serviceClass)
                    cnt++;
            }
    }
    return cnt;
}

} // namespace jdc
