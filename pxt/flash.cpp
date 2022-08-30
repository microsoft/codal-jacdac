#include "pxt.h"
#include "Flash.h"
#include "CodalJacdac.h"

namespace settings {
uintptr_t largeStoreStart();
size_t largeStoreSize();
CODAL_FLASH *largeStoreFlash();
} // namespace settings

static jacscriptmgr_cfg_t cfg;

extern "C" {
void flash_program(void *dst, const void *src, uint32_t len) {
    JD_ASSERT(cfg.program_base != NULL);
    CODAL_FLASH *flash = settings::largeStoreFlash();

    int pageSize = flash->pageSize((uintptr_t)dst);
    uint32_t pageMask = pageSize - 1;

    ptrdiff_t diff = (uintptr_t)dst - (uintptr_t)cfg.program_base;
    JD_ASSERT(0 <= diff && diff + len <= cfg.max_program_size);

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
}

void flash_erase(void *dst) {
    JD_ASSERT(cfg.program_base != NULL);

    ptrdiff_t diff = (uintptr_t)dst - (uintptr_t)cfg.program_base;
    JD_ASSERT(0 <= diff && (uintptr_t)diff <= cfg.max_program_size - JD_FLASH_PAGE_SIZE);
    JD_ASSERT((diff & (JD_FLASH_PAGE_SIZE - 1)) == 0);

    CODAL_FLASH *flash = settings::largeStoreFlash();
    int pageSize = flash->pageSize((uintptr_t)dst);
    JD_ASSERT(pageSize <= JD_FLASH_PAGE_SIZE);

    uintptr_t off = (uintptr_t)dst;
    uintptr_t endoff = off + JD_FLASH_PAGE_SIZE;
    while (off < endoff) {
        flash->erasePage(off);
        off += pageSize;
    }
}

void init_jacscript_manager(void) {
    cfg.max_program_size = settings::largeStoreSize();
    cfg.program_base = (void *)(settings::largeStoreStart());
    if (!cfg.program_base)
        target_panic(PANIC_JACDAC);
    JD_ASSERT(((uintptr_t)cfg.program_base & (JD_FLASH_PAGE_SIZE - 1)) == 0);
    jacscriptmgr_init(&cfg);
}

void flash_sync(void) {}
}
