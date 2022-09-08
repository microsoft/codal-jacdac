#include "jdc.h"

#ifdef IS_MICROBIT
#include "MicroBitRadio.h"
#include "MicroBitDevice.h"

#include "jacdac/dist/c/bitradio.h"

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

// payload: number (9 ... 12)
#define PACKET_TYPE_NUMBER 0
// payload: number (9 ... 12), name length (13), name (14 ... 26)
#define PACKET_TYPE_VALUE 1
// payload: string length (9), string (10 ... 28)
#define PACKET_TYPE_STRING 2
// payload: buffer length (9), buffer (10 ... 28)
#define PACKET_TYPE_BUFFER 3
// payload: number (9 ... 16)
#define PACKET_TYPE_DOUBLE 4
// payload: number (9 ... 16), name length (17), name (18 ... 26)
#define PACKET_TYPE_DOUBLE_VALUE 5

// Packet Spec:
// | 0              | 1 ... 4       | 5 ... 8           | 9 ... 28
// ----------------------------------------------------------------
// | packet type    | system time   | serial number     | payload
//
// Serial number defaults to 0 unless enabled by user

void bitradio_process(srv_t *state) {
    if (state->radio) {
        uint8_t buf[32];
        int len = state->radio->datagram.recv(buf, sizeof(buf));
        if (len >= 9) {
            if (buf[0] == PACKET_TYPE_DOUBLE || buf[0] == PACKET_TYPE_NUMBER) {
                double v;
                if (buf[0] == PACKET_TYPE_NUMBER) {
                    int q;
                    memcpy(&q, buf + 9, 4);
                    v = q;
                } else {
                    memcpy(&v, buf + 9, 8);
                }
                jd_send(state->service_index, JD_BIT_RADIO_CMD_NUMBER_RECEIVED, &v, 8);
            }
        }
    }
}

static void send_packet(srv_t *state, int tp, const void *data, unsigned datasize) {
    if (!state->radio)
        return;
    uint8_t buf[28];
    buf[0] = tp;
    memcpy(buf + 1, &now_ms, 4);
    uint32_t serial = microbit_serial_number();
    memcpy(buf + 5, &serial, 4);
    if (datasize > 20)
        datasize = 20;
    memcpy(buf + 9, data, datasize);
    state->radio->datagram.send(buf, datasize + 9);
}

void bitradio_handle_packet(srv_t *state, jd_packet_t *pkt) {
    switch (pkt->service_command) {
    case JD_BIT_RADIO_CMD_SEND_NUMBER: {
        double v;
        memcpy(&v, pkt->data, sizeof(v));
        int vi = (int)v;
        if (vi == v)
            send_packet(state, PACKET_TYPE_NUMBER, &vi, 4);
        else
            send_packet(state, PACKET_TYPE_DOUBLE, &v, 8);
        break;
    }
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
