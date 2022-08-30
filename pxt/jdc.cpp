#include "pxt.h"
#include "CodalJacdac.h"
#include "ZSingleWireSerial.h"

#if defined(NRF52833_XXAA)
#define IS_MICROBIT 1
#endif

namespace jdc {

void bridge_init();

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
    bridge_init();
}

} // namespace jdc

extern "C" void init_local_services(void) {
#ifdef IS_MICROBIT
    button_init(14, 0, NO_PIN);
    button_init(23, 0, NO_PIN);
#endif
}
