#pragma once

#include <stddef.h>
#include "../model/MonitorEvent.h"

namespace layertime { namespace observation_log {
extern const char *const kPath;
extern const char *const kHeader;
constexpr size_t kRowBufferSize = 640;
struct Row {
    const char *producer = "";
    const char *bootId = "";
    const char *recordedLocalTime = ""; // Recorder's clock; not RF capture time.
    const char *detectorName = "";
    bool repeat = false;
    MonitorEvent event;
};
// A complete row without a newline, or 0 with an empty output on failure.
// Quotes are doubled; backslashes/control bytes are reversibly escaped.
// A missing trustworthy producer wall clock is an empty timestamp field.
size_t format(const Row &row, char *out, size_t size);
} }
