#include "jdc.h"

#ifdef IS_MICROBIT
#include "MicroBitThermometer.h"
#include "MicroBitAccelerometer.h"
#include "MicroBitI2C.h"
#include "MicroBitDisplay.h"
#include "NRF52TouchSensor.h"
#include "MicroBitAudio.h"
#include "MicroBitDevice.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static MicroBitI2C *get_i2c(void) {
    static MicroBitI2C *i2c;
    if (!i2c)
        i2c = new MicroBitI2C(*GETPIN(MICROBIT_PIN_INT_SDA), *GETPIN(MICROBIT_PIN_INT_SCL));
    return i2c;
}

static Accelerometer *accel;
static MicroBitThermometer *mb_thermometer_dev;

static void mb_thermometer_init(void) {
    if (mb_thermometer_dev)
        return;
    mb_thermometer_dev = new MicroBitThermometer();
}
static void mb_thermometer_process(void) {}
static void *mb_thermometer_get_reading(void) {
    static env_reading_t v;
    int t = mb_thermometer_dev->getTemperature();
    v.value = t << 10;
    // guess-work...
    v.error = 1 << 10;
    v.min_value = -20 * 1024;
    v.max_value = 80 << 10;
    return &v;
}

static void mb_accel_init(void) {
    if (accel)
        return;
    accel = &MicroBitAccelerometer::autoDetect(*get_i2c());
}
static void mb_accel_process(void) {}
static void *mb_accel_get_reading(void) {
    Sample3D s = accel->getSample();
    static int32_t res[3];
    res[0] = s.x << 10;
    res[1] = s.y << 10;
    res[2] = s.z << 10;
    return res;
}

const accelerometer_api_t mb_accel = {mb_accel_init, mb_accel_process, NULL, mb_accel_get_reading};
const env_sensor_api_t mb_thermometer = {mb_thermometer_init, mb_thermometer_process, NULL,
                                         mb_thermometer_get_reading};

extern "C" void accelerometer_data_transform(int32_t sample[3]) {}

static const uint8_t row_pins[5] = {21, 22, 15, 24, 19};
static const uint8_t col_pins[5] = {28, 11, 31, P1_5, 30};

static const MatrixPoint ledMatrixPositions[5 * 5] = {
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {1, 1}, {1, 2}, {1, 3},
    {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {3, 1}, {3, 2},
    {3, 3}, {3, 4}, {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}};

static MicroBitDisplay *display;
static uint8_t disp_br = 255;
static uint8_t disp_buf[NUM_DISPLAY_COLS];

static void disp_init() {
    if (display)
        return;

    static NRF52Pin *ledRowPins[5];
    static NRF52Pin *ledColPins[5];

    for (int i = 0; i < 5; ++i) {
        ledRowPins[i] = GETPIN(row_pins[i]);
        ledColPins[i] = GETPIN(col_pins[i]);
    }

    // Bring up our display pins as high drive.
    for (NRF52Pin *p : ledRowPins)
        p->setHighDrive(true);

    for (NRF52Pin *p : ledColPins)
        p->setHighDrive(true);

    static MatrixMap ledMatrixMap = {
        5, 5, 5, 5, (Pin **)ledRowPins, (Pin **)ledColPins, ledMatrixPositions};

    display = new MicroBitDisplay(ledMatrixMap);
}

void disp_refresh(void) {
    disp_init();
    for (int i = 0; i < NUM_DISPLAY_COLS; ++i) {
        for (int j = 0; j < NUM_DISPLAY_ROWS; ++j) {
            display->image.setPixelValue(i, j, disp_buf[i] & (1 << j) ? disp_br : 0);
        }
    }
}

void disp_show(uint8_t *img) {
    memcpy(disp_buf, img, sizeof(disp_buf));
    disp_refresh();
}

void disp_set_brigthness(uint16_t v) {
    disp_br = v >> 8;
    disp_refresh();
}

extern "C" void bitradio_init(void);

static NRF52Serial *serial_;
void platformSendSerial(const char *data, int len) {
    if (!serial_)
        serial_ = new NRF52Serial(*GETPIN(USB_UART_TX), *GETPIN(USB_UART_RX), NRF_UARTE0);
    serial_->send((uint8_t *)data, len);
}

void flush_dmesg(Event) {
#if DEVICE_DMESG_BUFFER_SIZE > 0
    int p = codalLogStore.ptr;
    if (p > 0) {
        platformSendSerial(codalLogStore.buffer, p);

        target_disable_irq();
        int newp = codalLogStore.ptr - p;
        if (newp < 0)
            newp = 0;
        if (newp)
            memmove(codalLogStore.buffer, codalLogStore.buffer + p, newp);
        codalLogStore.ptr = newp;
        target_enable_irq();
    }
#endif
}

extern "C" void platform_panic(int error_code) {
    target_disable_irq();
    DMESG("JD PANIC %d", error_code);
    int p = codalLogStore.ptr;
    if (serial_ && p > 0) {
        nrf_uarte_tx_buffer_set(NRF_UARTE0, (uint8_t *)codalLogStore.buffer, p);
        nrf_uarte_task_trigger(NRF_UARTE0, NRF_UARTE_TASK_STARTTX);
        target_wait_us(p * 100);
    }
    codal::microbit_panic(error_code);
}

static int read_button(void *btn) {
    return ((TouchButton *)btn)->isPressed();
}
void add_touch_button(int pinid) {
    button_init_fn(read_button, new TouchButton(*GETPIN(pinid), *NRF52Pin::touchSensor, 3500));
}

extern "C" const char *app_get_instance_name(int service_idx) {
    switch (service_idx - 2) {
    case 1:
        return "A";
    case 2:
        return "B";
    case 3:
        return "logo";
    case 4:
        return "P0";
    case 5:
        return "P1";
    case 6:
        return "P2";
    }
    return NULL;
}

extern "C" void soundlevel_init(void);
extern "C" void bitradio_init(void);
extern "C" void soundplayer_init(MicroBitAudio *audio);

#endif

extern "C" void init_local_services(void) {
#ifdef IS_MICROBIT
#define DEVICE_ID_CYCLE DEVICE_ID_JACDAC_CONTROL_SERVICE
    pxt::setSendToUART(platformSendSerial);
    EventModel::defaultEventBus->listen(DEVICE_ID_CYCLE, 200, flush_dmesg,
                                        MESSAGE_BUS_LISTENER_DROP_IF_BUSY);
    system_timer_event_every_us(100000, DEVICE_ID_CYCLE, 200);

    auto irq1 = GETPIN(25);
    irq1->getDigitalValue();
    irq1->setPull(PullMode::Up);
    irq1->setActiveLo();

    auto capTimer = new NRFLowLevelTimer(NRF_TIMER3, TIMER3_IRQn);
    NRF52Pin::touchSensor = new NRF52TouchSensor(*capTimer);

    auto adcTimer = new NRFLowLevelTimer(NRF_TIMER2, TIMER2_IRQn);
    NRF52Pin::adc = new NRF52ADC(*adcTimer, 91);

    auto audio = new MicroBitAudio(*GETPIN(2), *GETPIN(0));
    audio->setSpeakerEnabled(true);
    audio->setPinEnabled(false);

    button_init(BUTTONA, 0, NO_PIN);
    button_init(BUTTONB, 0, NO_PIN);
    add_touch_button(P1_04); // logo
    add_touch_button(2);     // P0
    add_touch_button(3);     // P1
    add_touch_button(4);     // P2

    temperature_init(&mb_thermometer);
    accelerometer_init(&mb_accel);
    soundlevel_init();

    bitradio_init();

    dotmatrix_init();
    soundplayer_init(audio);
    cbuzzer_init(&audio->virtualOutputPin);

    NVIC_SetPriority(TIMER1_IRQn, 7);       // System timer (general purpose)
    NVIC_SetPriority(TIMER2_IRQn, 5);       // ADC timer.
    NVIC_SetPriority(TIMER3_IRQn, 3);       // Cap touch.
    NVIC_SetPriority(TIMER4_IRQn, 3);       // Display and Light Sensing.
    NVIC_SetPriority(SAADC_IRQn, 5);        // Analogue to Digital Converter (microphone etc)
    NVIC_SetPriority(PWM0_IRQn, 5);         // General Purpose PWM on edge connector
    NVIC_SetPriority(PWM1_IRQn, 4);         // PCM audio on speaker (high definition sound)
    NVIC_SetPriority(PWM2_IRQn, 3);         // Waveform Generation (neopixel)
    NVIC_SetPriority(RADIO_IRQn, 4);        // Packet radio
    NVIC_SetPriority(UARTE0_UART0_IRQn, 2); // Serial port
    NVIC_SetPriority(GPIOTE_IRQn, 2);       // Pin interrupt events

#endif
}
