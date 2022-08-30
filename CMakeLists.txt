project(codal-jacdac)

include("${CODAL_UTILS_LOCATION}")

# find sources and headers
RECURSIVE_FIND_FILE(SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source" "*.c??")

list(APPEND INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/jacdac-c"
    "${CMAKE_CURRENT_SOURCE_DIR}/jacdac-c/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/jacdac-c/jacscript"
)

list(APPEND SOURCE_FILES
    "jacdac-c/source/jd_control.c"
    "jacdac-c/source/jd_ipipe.c"
    "jacdac-c/source/jd_opipe.c"
    "jacdac-c/source/jd_physical.c"
    "jacdac-c/source/jd_protocol.c"
    "jacdac-c/source/jd_queue.c"
    "jacdac-c/source/jd_bqueue.c"
    "jacdac-c/source/jd_send_util.c"
    "jacdac-c/source/jd_services.c"
    "jacdac-c/source/jd_util.c"
    "jacdac-c/source/jd_usb.c"
    "jacdac-c/source/interfaces/event_queue.c"
    "jacdac-c/source/interfaces/simple_rx.c"
    "jacdac-c/source/interfaces/tx_queue.c"
    "jacdac-c/services/jd_status_light.c"
    "jacdac-c/services/power.c"
    "jacdac-c/services/button.c"
    "jacdac-c/services/temperature.c"
    "jacdac-c/services/jd_sensor.c"
    "jacdac-c/services/jd_env.c"
    "jacdac-c/client/rolemgr.c"
    "jacdac-c/client/routing.c"
    "jacdac-c/jacscript/aggbuffer.c"
    "jacdac-c/jacscript/buffer.c"
    "jacdac-c/jacscript/fibers.c"
    "jacdac-c/jacscript/jacscript.c"
    "jacdac-c/jacscript/jacscriptmgr.c"
    "jacdac-c/jacscript/jdiface.c"
    "jacdac-c/jacscript/regcache.c"
    "jacdac-c/jacscript/stmt.c"
    "jacdac-c/jacscript/expr.c"
    "jacdac-c/jacscript/strformat.c"
    "jacdac-c/jacscript/value.c"
    "jacdac-c/jacscript/verify.c"
    "jacdac-c/jacscript/jacscloud.c"
    "jacdac-c/jacscript/tsagg.c"
    "jacdac-c/jacdac/dist/c/jd_spec_pack.c"
    "jacdac-c/storage/ff/ff.c"
    "jacdac-c/storage/lstore.c"
    "jacdac-c/storage/crc32.c"
)

include_directories(${INCLUDE_DIRS})
add_library(codal-jacdac ${SOURCE_FILES})

target_link_libraries(
    codal-jacdac
    codal-core
)

target_include_directories(codal-jacdac PUBLIC ${INCLUDE_DIRS})
