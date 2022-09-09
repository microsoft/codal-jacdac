#include "pxt.h"
#include "CodalJacdac.h"

#if defined(NRF52833_XXAA)
#define IS_MICROBIT 1
#endif

#define GETPIN pxt::getPin

extern "C" void cbuzzer_init(Pin *pin);
