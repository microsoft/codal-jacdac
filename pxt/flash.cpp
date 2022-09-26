#include "pxt.h"
#include "Flash.h"
#include "CodalJacdac.h"

#define MAX_PROG_KB 4

#define USE_LARGE_STORE_FLASH 0
#define USE_SETTINGS 1

namespace settings {
uintptr_t largeStoreStart();
size_t largeStoreSize();
CODAL_FLASH *largeStoreFlash();
Buffer _get(String key);
int _remove(String key);
int _set(String key, Buffer data);
} // namespace settings

static jacscriptmgr_cfg_t cfg;

#if USE_SETTINGS
PXT_DEF_STRING(settingsKey, "_jacs_prog")
#define SETTINGS_KEY (String)(void *) settingsKey
static unsigned max_addr;
#endif

extern "C" {
void flash_program(void *dst, const void *src, uint32_t len) {
    JD_ASSERT(cfg.program_base != NULL);
    ptrdiff_t diff = (uintptr_t)dst - (uintptr_t)cfg.program_base;
    JD_ASSERT(0 <= diff && diff + len <= cfg.max_program_size);

#if USE_LARGE_STORE_FLASH
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
#if USE_SETTINGS
    if (diff + len > max_addr) {
        max_addr = diff + len;
    }
#endif
#endif
}

void flash_erase(void *dst) {
    JD_ASSERT(cfg.program_base != NULL);
    ptrdiff_t diff = (uintptr_t)dst - (uintptr_t)cfg.program_base;
    JD_ASSERT(0 <= diff && (uintptr_t)diff <= cfg.max_program_size - JD_FLASH_PAGE_SIZE);
    JD_ASSERT((diff & (JD_FLASH_PAGE_SIZE - 1)) == 0);

#if USE_LARGE_STORE_FLASH
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
#if USE_SETTINGS
    if (diff == 0)
        max_addr = 0;
#endif
#endif
}

void flash_sync(void) {
#if USE_SETTINGS
    settings::_remove(SETTINGS_KEY);
    auto buf = mkBuffer(cfg.program_base, max_addr);
    settings::_set(SETTINGS_KEY, buf);
#endif
}

void init_jacscript_manager(void) {
#if USE_LARGE_STORE_FLASH
    cfg.max_program_size = settings::largeStoreSize();
    cfg.program_base = (void *)(settings::largeStoreStart());
    if (!cfg.program_base)
        target_panic(PANIC_JACDAC);
    JD_ASSERT(((uintptr_t)cfg.program_base & (JD_FLASH_PAGE_SIZE - 1)) == 0);
#else
    cfg.max_program_size = MAX_PROG_KB * 1024;
    cfg.program_base = jd_alloc(cfg.max_program_size);
#if USE_SETTINGS
    auto buf = settings::_get(SETTINGS_KEY);
    if (buf) {
        unsigned len = buf->length;
        if (len > cfg.max_program_size)
            len = cfg.max_program_size;
        memcpy(cfg.program_base, buf->data, len);
    }
#endif
#endif
    jacscriptmgr_init(&cfg);
}
}
