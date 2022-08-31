#pragma once

extern "C" {
#include "jd_client.h"
#include "jacscript.h"
#include "jd_drivers.h"
#include "services/jd_services.h"
#include "services/interfaces/jd_disp.h"
}

#include "codal_target_hal.h"
#include "DMASingleWireSerial.h"
#include "MessageBus.h"
#include "Timer.h"
#include "CodalDmesg.h"

#define PANIC_JACDAC 777

void jdhw_init(codal::DMASingleWireSerial *dmasws);
