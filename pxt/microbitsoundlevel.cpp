#include "jdc.h"

#ifdef IS_MICROBIT

#include "LevelDetectorSPL.h"
#include "StreamNormalizer.h"

#include "jacdac/dist/c/soundlevel.h"

struct srv_state {
    SENSOR_COMMON;
    uint16_t laud_threshold;
    uint16_t quiet_threshold;
    uint8_t pin_runmic;
    uint8_t pin_mic;
    uint16_t sample;
    uint32_t block_laud;
    uint32_t block_quiet;
    NRF52ADCChannel *channel;
    LevelDetectorSPL *level;
};

REG_DEFINITION(                                  //
    soundlevel_regs,                             //
    REG_SENSOR_COMMON,                           //
    REG_U16(JD_SOUND_LEVEL_REG_LOUD_THRESHOLD),  //
    REG_U16(JD_SOUND_LEVEL_REG_QUIET_THRESHOLD), //
)

static void soundlevel_sync_regs(srv_t *state) {
    if (!state->got_query) {
        pin_set(state->pin_runmic, 0);
        if (state->channel)
            state->channel->disable();
        return;
    }

    if (!state->level) {
        state->channel = NRF52Pin::adc->getChannel(*GETPIN(state->pin_mic));
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

        if (sensor_should_send_threshold_event(&state->block_laud, 50,
                                               state->sample >= state->laud_threshold))
            jd_send_event(state, JD_SOUND_LEVEL_EV_LOUD);

        if (sensor_should_send_threshold_event(&state->block_quiet, 50,
                                               state->sample <= state->quiet_threshold))
            jd_send_event(state, JD_SOUND_LEVEL_EV_QUIET);
    } else {
        state->sample = 0;
    }

    sensor_process_simple(state, &state->sample, sizeof(state->sample));
}

void soundlevel_handle_packet(srv_t *state, jd_packet_t *pkt) {
    if (service_handle_register(state, pkt, soundlevel_regs))
        return;

    if (sensor_handle_packet_simple(state, pkt, &state->sample, sizeof(state->sample)))
        soundlevel_sync_regs(state);
}

SRV_DEF(soundlevel, JD_SERVICE_CLASS_SOUND_LEVEL);
extern "C" void soundlevel_init(void) {
    SRV_ALLOC(soundlevel);

    state->pin_runmic = 20;
    state->pin_mic = 5;
    state->quiet_threshold = 0x1e1e; // 60
    state->laud_threshold = 0x5696;  // 75

    pin_set(state->pin_runmic, 0);
    GETPIN(state->pin_runmic)->setHighDrive(true);
}

#endif
