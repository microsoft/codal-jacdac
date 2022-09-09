// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "jdc.h"

#include "jacdac/dist/c/buzzer.h"

struct srv_state {
    SRV_COMMON;
    uint8_t volume;
    Pin *pin;
    uint32_t end_tone_time;
    uint16_t period;
};

REG_DEFINITION(               //
    cbuzzer_regs,             //
    REG_SRV_COMMON,           //
    REG_U8(JD_REG_INTENSITY), //
)

static void play_tone(srv_t *state, uint32_t period, uint32_t duty) {
    duty = (duty * state->volume) >> 8;
    state->pin->setAnalogPeriodUs(period);
    int v = (1024 * duty) / period;
    if (v < 0)
        v = 0;
    if (v > 1023)
        v = 1023;
    state->pin->setAnalogValue(v);
}

void cbuzzer_process(srv_t *state) {
    if (state->period && in_past(state->end_tone_time)) {
        state->period = 0;
        state->pin->setAnalogValue(0);
    }
}

void cbuzzer_handle_packet(srv_t *state, jd_packet_t *pkt) {
    switch (pkt->service_command) {
    case JD_BUZZER_CMD_PLAY_TONE:
        // ensure input is big enough
        if (pkt->service_size >= 6) {
            jd_buzzer_play_tone_t *d = (jd_buzzer_play_tone_t *)pkt->data;
            state->end_tone_time = now + d->duration * 1000;
            state->period = d->period;
            play_tone(state, state->period, d->duty);
        }
        cbuzzer_process(state);
        break;
    default:
        service_handle_register_final(state, pkt, cbuzzer_regs);
        break;
    }
}

SRV_DEF(cbuzzer, JD_SERVICE_CLASS_BUZZER);
void cbuzzer_init(Pin *pin) {
    SRV_ALLOC(cbuzzer);
    state->pin = pin;
    state->volume = 255;
}
