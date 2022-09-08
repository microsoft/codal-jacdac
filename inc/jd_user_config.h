#include "CodalDmesg.h"

#if defined(NRF52832_XXAA) || defined(NRF52832_XXAB) || defined(NRF52833_XXAA) ||                  \
    defined(NRF52840_XXAA)
#ifndef NRF52_SERIES
#define NRF52_SERIES
#endif
#endif

#define JD_LOG DMESG

#define JD_WR_OVERHEAD 45

#define JD_CLIENT 1

#define JD_RAW_FRAME 1

// this is min. erase size
#define JD_FLASH_PAGE_SIZE 4096 // TODO

// probably not so useful on brains...
#define JD_CONFIG_WATCHDOG 0

// #define JD_USB_BRIDGE 1

#define JD_SEND_FRAME_SIZE 1024

#define JD_CONFIG_STATUS 0

#ifdef NRF52_SERIES
#define JD_BRIDGE 1
int mbbridge_send_frame(void *frame);
#define JD_BRIDGE_SEND(f) mbbridge_send_frame(f)
#endif

#define NUM_DISPLAY_COLS 5
#define NUM_DISPLAY_ROWS 5

#define JD_INSTANCE_NAME 1

#define JD_RX_QUEUE_SIZE 1024
