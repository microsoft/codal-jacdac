#include "jdc.h"

#ifdef IS_MICROBIT
#include "MicroBitAudio.h"

#include "jacdac/dist/c/soundplayer.h"

struct srv_state {
    SRV_COMMON;
    uint16_t volume;
    uint8_t soundptr;
    MicroBitAudio *audio;
    jd_opipe_desc_t list_pipe;
};

REG_DEFINITION(                          //
    soundplayer_regs,                    //
    REG_SRV_COMMON,                      //
    REG_U16(JD_SOUND_PLAYER_REG_VOLUME), //
)

static const char *sounds[] = {
    "giggle", "happy",   "hello",  "mysterious", "sad",
    "slide",  "soaring", "spring", "twinkle",    "yawn",
};
#define NUM_SOUNDS (int)(sizeof(sounds) / sizeof(sounds[0]))

static void soundplayer_sync_regs(srv_t *state) {
    state->audio->setVolume(state->volume >> 8);
}

void soundplayer_process(srv_t *state) {
    while (state->soundptr < NUM_SOUNDS) {
        unsigned sz = 4 + strlen(sounds[state->soundptr]);
        int err = jd_opipe_check_space(&state->list_pipe, sz);
        if (err == JD_PIPE_TRY_AGAIN)
            break;
        if (err != 0) {
            state->soundptr = NUM_SOUNDS;
            jd_opipe_close(&state->list_pipe);
            break;
        }

        char buf[sz + 1];
        uint32_t duration = 100; // TODO?
        memcpy(buf, &duration, 4);
        strcpy(buf + 4, sounds[state->soundptr]);
        err = jd_opipe_write(&state->list_pipe, buf, sz);
        JD_ASSERT(err == 0);

        state->soundptr++;
        if (state->soundptr >= NUM_SOUNDS)
            jd_opipe_close(&state->list_pipe);
    }
}

void soundplayer_handle_packet(srv_t *state, jd_packet_t *pkt) {
    switch (pkt->service_command) {
    case JD_SOUND_PLAYER_CMD_CANCEL:
        state->audio->soundExpressions.stop();
        break;
    case JD_SOUND_PLAYER_CMD_PLAY:
        for (int i = 0; i < NUM_SOUNDS; ++i)
            if (pkt->service_size == strlen(sounds[i]) && memcmp(pkt->data, sounds[i], pkt->service_size) == 0) {
                state->audio->soundExpressions.stop();
                state->audio->soundExpressions.playAsync(sounds[i]);
                break;
            }
        break;

    case JD_SOUND_PLAYER_CMD_LIST_SOUNDS:
        if (jd_opipe_open_cmd(&state->list_pipe, pkt) == 0)
            state->soundptr = 0;
        break;

    default:
        if (service_handle_register_final(state, pkt, soundplayer_regs) > 0)
            soundplayer_sync_regs(state);
        break;
    }
}

SRV_DEF(soundplayer, JD_SERVICE_CLASS_SOUND_PLAYER);
extern "C" void soundplayer_init(MicroBitAudio *audio) {
    SRV_ALLOC(soundplayer);
    state->audio = audio;
    state->volume = 0x8000;
    state->soundptr = NUM_SOUNDS;
    soundplayer_sync_regs(state);
}

#endif
