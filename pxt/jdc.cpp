#include "pxt.h"
#include "CodalJacdac.h"
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

} // namespace jdc
