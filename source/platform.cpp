#include "CodalJacdac.h"
#include "CodalHeapAllocator.h"

using namespace codal;

extern "C" {

uint32_t now;

void jd_alloc_init() {}
void jd_alloc_stack_check() {}

void *jd_alloc(uint32_t size) {
    if (size == 0)
        size = 1;
    void *r = device_malloc(size);
    if (!r)
        target_panic(20);
    memset(r, 0, size);
    return r;
}

void jd_free(void *ptr) {
    if (ptr)
        device_free(ptr);
}

uint8_t jd_connected_blink;
void jd_blink(uint8_t encoded) {}
void jd_glow(uint32_t glow) {}

void pwr_enter_no_sleep(void) {}
void pwr_leave_no_sleep(void) {}

int target_in_irq(void) {
    return 0;
    //    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

uint64_t hw_device_id(void) {
    return target_get_serial();
}
}