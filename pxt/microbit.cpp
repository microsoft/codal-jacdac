#include "pxt.h"
#include "CodalJacdac.h"

#if defined(NRF52833_XXAA)
#define IS_MICROBIT 1
#endif

#undef PIN
#define PIN pxt::getPin

#ifdef IS_MICROBIT
#include "MicroBitThermometer.h"
#include "MicroBitAccelerometer.h"
#include "MicroBitI2C.h"
#include "MicroBitDisplay.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static MicroBitI2C *get_i2c(void) {
    static MicroBitI2C *i2c;
    if (!i2c)
        i2c = new MicroBitI2C(*PIN(MICROBIT_PIN_INT_SDA), *PIN(MICROBIT_PIN_INT_SCL));
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
        ledRowPins[i] = PIN(row_pins[i]);
        ledColPins[i] = PIN(col_pins[i]);
    }

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

#endif

extern "C" void init_local_services(void) {
#ifdef IS_MICROBIT
    button_init(BUTTONA, 0, NO_PIN);
    button_init(BUTTONB, 0, NO_PIN);
    temperature_init(&mb_thermometer);
    accelerometer_init(&mb_accel);
    dotmatrix_init();
#endif
}
