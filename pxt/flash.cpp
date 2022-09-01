#include "pxt.h"
#include "Flash.h"
#include "CodalJacdac.h"

#define FLASH 0

namespace settings {
uintptr_t largeStoreStart();
size_t largeStoreSize();
CODAL_FLASH *largeStoreFlash();
} // namespace settings

static jacscriptmgr_cfg_t cfg;

extern "C" {
void flash_program(void *dst, const void *src, uint32_t len) {
    JD_ASSERT(cfg.program_base != NULL);
    ptrdiff_t diff = (uintptr_t)dst - (uintptr_t)cfg.program_base;
    JD_ASSERT(0 <= diff && diff + len <= cfg.max_program_size);

#if FLASH
    CODAL_FLASH *flash = settings::largeStoreFlash();

    int pageSize = flash->pageSize((uintptr_t)dst);
    uint32_t pageMask = pageSize - 1;

    uintptr_t off = (uintptr_t)dst;
    while (len > 0) {
        unsigned n = pageSize - (off & pageMask);
        if (n > len)
            n = len;
        flash->writeBytes(off, src, n);
        off += n;
        src = (const uint8_t *)src + n;
        len -= n;
    }
#else
    JD_ASSERT(((uintptr_t)src & 3) == 0);
    JD_ASSERT((diff & 7) == 0);
    for (unsigned i = 0; i < len; ++i)
        JD_ASSERT(((uint8_t *)dst)[i] == 0xff);
    memcpy(dst, src, len);
#endif
}

void flash_erase(void *dst) {
    JD_ASSERT(cfg.program_base != NULL);
    ptrdiff_t diff = (uintptr_t)dst - (uintptr_t)cfg.program_base;
    JD_ASSERT(0 <= diff && (uintptr_t)diff <= cfg.max_program_size - JD_FLASH_PAGE_SIZE);
    JD_ASSERT((diff & (JD_FLASH_PAGE_SIZE - 1)) == 0);

#if FLASH
    CODAL_FLASH *flash = settings::largeStoreFlash();
    int pageSize = flash->pageSize((uintptr_t)dst);
    JD_ASSERT(pageSize <= JD_FLASH_PAGE_SIZE);

    uintptr_t off = (uintptr_t)dst;
    uintptr_t endoff = off + JD_FLASH_PAGE_SIZE;
    while (off < endoff) {
        flash->erasePage(off);
        off += pageSize;
    }
#else
    memset(dst, 0xff, JD_FLASH_PAGE_SIZE);
#endif
}

void init_jacscript_manager(void) {
#if FLASH
    cfg.max_program_size = settings::largeStoreSize();
    cfg.program_base = (void *)(settings::largeStoreStart());
    if (!cfg.program_base)
        target_panic(PANIC_JACDAC);
    JD_ASSERT(((uintptr_t)cfg.program_base & (JD_FLASH_PAGE_SIZE - 1)) == 0);
#else
    cfg.max_program_size = 4 * 1024;
    cfg.program_base = jd_alloc(cfg.max_program_size);
#endif
    jacscriptmgr_init(&cfg);
}

void flash_sync(void) {}
}
