#include "jdc.h"

#ifdef IS_MICROBIT

#include "LevelDetectorSPL.h"
#include "StreamNormalizer.h"

#include "jacdac/dist/c/soundlevel.h"

struct srv_state {
    SENSOR_COMMON;
    uint8_t pin_runmic;
    uint8_t pin_mic;
    uint16_t sample;
    NRF52ADCChannel *channel;
    LevelDetectorSPL *level;
};

static void soundlevel_sync_regs(srv_t *state) {
    if (!state->got_query) {
        pin_set(state->pin_runmic, 0);
        if (state->channel)
            state->channel->disable();
        return;
    }

    if (!state->level) {
        DMESG("SL enable");
        state->channel = NRF52Pin::adc->getChannel(*PIN(state->pin_mic));
        auto norm =
            new StreamNormalizer(state->channel->output, 1.0f, true, DATASTREAM_FORMAT_UNKNOWN, 10);
        state->level = new LevelDetectorSPL(norm->output, 75.0, 60.0, 9, 52, DEVICE_ID_MICROPHONE);
        state->channel->setGain(7, 0);
    }

    pin_set(state->pin_runmic, 1);
    state->channel->enable();
}

#define MICROPHONE_MIN 52.0f
#define MICROPHONE_MAX 120.0f

void soundlevel_process(srv_t *state) {
    if (state->got_query) {
        float v = state->level->getValue() - MICROPHONE_MIN;
        v = v * (0xffff / (MICROPHONE_MAX - MICROPHONE_MIN));
        if (v < 0)
            v = 0;
        if (v > 0xffff)
            v = 0xffff;
        state->sample = (uint16_t)v;
    } else {
        state->sample = 0;
    }

    sensor_process_simple(state, &state->sample, sizeof(state->sample));
}

void soundlevel_handle_packet(srv_t *state, jd_packet_t *pkt) {
    if (sensor_handle_packet_simple(state, pkt, &state->sample, sizeof(state->sample)))
        soundlevel_sync_regs(state);
}

SRV_DEF(soundlevel, JD_SERVICE_CLASS_SOUND_LEVEL);
extern "C" void soundlevel_init(void) {
    SRV_ALLOC(soundlevel);

    state->pin_runmic = 20;
    state->pin_mic = 5;

    pin_set(state->pin_runmic, 0);
    PIN(state->pin_runmic)->setHighDrive(true);
}

#endif
