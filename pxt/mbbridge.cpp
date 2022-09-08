#include "jdc.h"

#ifdef NRF52_SERIES

namespace jdc {

struct ExchangeBuffer {
    uint8_t magic[8];
    uint8_t irqn;
    uint8_t padding[3];

    // only single buffer in each direction to ensure ordering
    volatile uint8_t recvBuf[256]; // the PC reads from here
    volatile uint8_t sendBuf[256]; // the PC writes here
};

static jd_queue_t usb_queue;
static ExchangeBuffer *buff;

static void logq_poke();

static volatile uint32_t *recvPtr() {
    auto bp = buff->recvBuf;
    if (bp[2])
        bp = NULL;
    return (volatile uint32_t *)bp;
}

static void pushOutData(const jd_frame_t *frame) {
    auto bp = recvPtr();
    if (!bp)
        target_panic(111);
    auto src = (uint32_t *)frame;
    int len = (JD_FRAME_SIZE(frame) + 3) >> 2;
    for (int i = 1; i < len; ++i)
        bp[i] = src[i];
    bp[0] = src[0]; // first word copied last to ensure atomicity
}

// a nice unused interrupt
extern "C" void TEMP_IRQHandler() {
    logq_poke();
}

extern "C" int mbbridge_send_frame(void *data) {
    auto frame = (jd_frame_t *)data;
    int r = -10;
    target_disable_irq();
    if (buff->recvBuf[2] != 0xff) {
        if (jd_queue_front(usb_queue) || !recvPtr()) {
            r = jd_queue_push(usb_queue, frame);
        } else {
            pushOutData(frame);
            r = 0;
        }
    }
    target_enable_irq();
    logq_poke();
    return r;
}

static void logq_poke() {
    target_disable_irq();
    jd_frame_t *f = jd_queue_front(usb_queue);
    if (f && recvPtr()) {
        pushOutData(f);
        jd_queue_shift(usb_queue);
    }
    if (buff->sendBuf[2]) {
        jd_frame_t *frame = (jd_frame_t *)buff->sendBuf;
        uint32_t declaredSize = JD_FRAME_SIZE(frame);
        uint16_t crc = jd_crc16((uint8_t *)frame + 2, declaredSize - 2);
        if (crc != frame->crc) {
            JD_LOG("USB crc err");
        } else {
            DMESG("sx %d", declaredSize);
            jd_send_frame_raw(frame);
        }
        buff->sendBuf[2] = 0;
    }
    target_enable_irq();
}

extern "C" uint32_t __StackTop;

void bridge_init() {
    // microbit_panic_timeout(0);
    buff = (ExchangeBuffer *)app_alloc(sizeof(*buff));
    memset(buff, 0, sizeof(*buff));
    buff->irqn = TEMP_IRQn;
    memcpy(buff->magic, "JDmx\xe9\xc0\xa6\xb0", 8);
    buff->recvBuf[2] = 0xff; // we'll wait until this is cleared by the computer

    NVIC_ClearPendingIRQ((IRQn_Type)buff->irqn);
    NVIC_EnableIRQ((IRQn_Type)buff->irqn);

    usb_queue = jd_queue_alloc(JD_USB_QUEUE_SIZE);

    // store the address to the top of the exchange buffer at the very bottom of the stack (which is
    // otherwise unused)
    (&__StackTop)[-1] = (uint32_t)buff;

    DMESG("started m:b bridge: %p", buff);
}

} // namespace jdc
#else
namespace jdc {
void bridge_init() {}
} // namespace jdc
#endif