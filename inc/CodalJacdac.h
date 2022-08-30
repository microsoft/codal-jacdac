#pragma once

extern "C" {
#include "jd_client.h"
}

#include "codal_target_hal.h"
#include "DMASingleWireSerial.h"
#include "MessageBus.h"
#include "Timer.h"
#include "CodalDmesg.h"

#define PANIC_JACDAC 777

void jdhw_init(codal::DMASingleWireSerial *dmasws);


