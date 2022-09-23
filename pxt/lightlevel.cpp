#include "jdc.h"
#include "jacdac/dist/c/lightlevel.h"

struct srv_state {
    SENSOR_COMMON;
    uint16_t sample;
    uint16_t (*read_value)(void);
};

void lightlevel_process(srv_t *state) {
    if (state->got_query) {
        state->sample = state->read_value();
    }
    sensor_process_simple(state, &state->sample, sizeof(state->sample));
}

void lightlevel_handle_packet(srv_t *state, jd_packet_t *pkt) {
    sensor_handle_packet_simple(state, pkt, &state->sample, sizeof(state->sample));
}

SRV_DEF(lightlevel, JD_SERVICE_CLASS_LIGHT_LEVEL);
extern "C" void lightlevel_init(uint16_t (*read_value)(void)) {
    SRV_ALLOC(lightlevel);
    state->streaming_interval = 100;
    state->read_value = read_value;
}
