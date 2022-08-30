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

#ifdef IS_MICROBIT
#include "MicroBitThermometer.h"

static MicroBitThermometer *mb_thermometer_dev;
static void mb_thermometer_init(void) {
    if (mb_thermometer_dev)
        return;
    mb_thermometer_dev = new MicroBitThermometer();
}
static void mb_thermometer_process(void) {}
static void *mb_thermometer_get_reading(void) {
    static env_reading_t v;
    int t = mb_thermometer_dev->getTemperature();
    v.value = t << 10;
    // guess-work...
    v.error = 1 << 10;
    v.min_value = -20 * 1024;
    v.max_value = 80 << 10;
    return &v;
}

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

const env_sensor_api_t mb_thermometer = {mb_thermometer_init, mb_thermometer_process, NULL,
                                         mb_thermometer_get_reading};
#endif

extern "C" void init_local_services(void) {
#ifdef IS_MICROBIT
    button_init(14, 0, NO_PIN);
    button_init(23, 0, NO_PIN);
    temperature_init(&mb_thermometer);
#endif
}
