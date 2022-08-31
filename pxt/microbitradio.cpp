#include "pxt.h"
#include "CodalJacdac.h"

#if defined(NRF52833_XXAA)
#define IS_MICROBIT 1
#endif

#ifdef IS_MICROBIT
#include "MicroBitRadio.h"

extern "C" {
#include "jacdac/dist/c/bitradio.h"
}

struct srv_state {
    SRV_COMMON;
    uint8_t enabled;
    uint8_t group;
    uint8_t tx_power;
    uint8_t freq_band;
    MicroBitRadio *radio;
};

REG_DEFINITION(                                  //
    bitradio_regs,                               //
    REG_SRV_COMMON,                              //
    REG_U8(JD_BIT_RADIO_REG_ENABLED),            //
    REG_U8(JD_BIT_RADIO_REG_GROUP),              //
    REG_U8(JD_BIT_RADIO_REG_TRANSMISSION_POWER), //
    REG_U8(JD_BIT_RADIO_REG_FREQUENCY_BAND),     //
)

void bitradio_process(srv_t *state) {}

static int clamp(int a, int v, int b) {
    if (v < a)
        return a;
    if (v > b)
        return b;
    return v;
}

static void bitradio_sync_regs(srv_t *state) {
    if (!state->enabled) {
        if (state->radio)
            state->radio->disable();
        return;
    }

    if (!state->radio)
        state->radio = new MicroBitRadio();

    state->radio->enable();

    state->tx_power = clamp(1, state->tx_power, 7);
    state->freq_band = clamp(0, state->freq_band, 83);

    state->radio->setTransmitPower(state->tx_power);
    state->radio->setFrequencyBand(state->freq_band);
    state->radio->setGroup(state->group);
}

void bitradio_handle_packet(srv_t *state, jd_packet_t *pkt) {
    switch (pkt->service_command) {
    case JD_BIT_RADIO_CMD_SEND_BUFFER:
    default:
        if (service_handle_register_final(state, pkt, bitradio_regs) > 0)
            bitradio_sync_regs(state);
        break;
    }
}

SRV_DEF(bitradio, JD_SERVICE_CLASS_BIT_RADIO);
extern "C" void bitradio_init(void) {
    SRV_ALLOC(bitradio);
    state->tx_power = 6;
    state->freq_band = 7;
}

#endif
