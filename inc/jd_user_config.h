#include "CodalDmesg.h"

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