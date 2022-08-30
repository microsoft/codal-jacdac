#include "pxt.h"
#include "CodalJacdac.h"
#include "ZSingleWireSerial.h"

namespace jdc {

/**
 * Start jacdac-c stack
 */
//%
void start() {
    // allow disabling SWS with the following anywhere in the TS code:
    // namespace userconfig { export const PIN_JACK_TX = 0xdead }
    if (getConfig(CFG_PIN_JACK_TX, 0) == 0xdead) {
        DMESG("Jacdac SWS disabled");
        return;
    }

    ZSingleWireSerial *sws;
#ifdef MICROBIT_CODAL
    sws = new ZSingleWireSerial(uBit.io.P12);
#else
    sws = new ZSingleWireSerial(*LOOKUP_PIN(JACK_TX));
#endif

    jdhw_init(sws);
}

} // namespace jdc