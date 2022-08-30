#include "CodalJacdac.h"
#include "CodalHeapAllocator.h"

using namespace codal;

namespace pxt {
Pin *getPin(int id);
}

extern "C" {

void pin_setup_output(int pin) {
    if ((uint8_t)pin != NO_PIN)
        pxt::getPin(pin)->setDigitalValue(0);
}

void pin_set(int pin, int v) {
    if ((uint8_t)pin != NO_PIN)
        pxt::getPin(pin)->setDigitalValue(v);
}

int pin_get(int pin) {
    if ((uint8_t)pin != NO_PIN)
        return pxt::getPin(pin)->getDigitalValue();
    return -1;
}

void pin_setup_input(int pin, int pull) {
    if ((uint8_t)pin == NO_PIN)
        return;
    pxt::getPin(pin)->getDigitalValue(pull < 0   ? PullMode::Down
                                      : pull > 0 ? PullMode::Up
                                                 : PullMode::None);
}

void pin_set_pull(int pin, int pull) {
    pxt::getPin(pin)->setPull(pull < 0 ? PullMode::Down : pull > 0 ? PullMode::Up : PullMode::None);
}

void pin_setup_analog_input(int pin) {
    if ((uint8_t)pin != NO_PIN)
        pxt::getPin(pin)->disconnect();
}
}