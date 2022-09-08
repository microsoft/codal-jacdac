#include "jdc.h"
#include "ZSingleWireSerial.h"

namespace jdc {

void bridge_init();

static ZSingleWireSerial *sws;

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
