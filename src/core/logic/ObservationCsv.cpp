#include "ObservationCsv.h"
#include <stdio.h>

namespace layertime { namespace observation_log {
const char *const kPath = "/recon_field_v1.csv";
const char *const kHeader = "schema,producer,boot_id,record,recorded_local_time,event_id,observed_uptime_ms,observed_unix_seconds,detector,confidence,source,source_id,detail,rssi,channel,band,count";
namespace {
class Writer {
public:
    Writer(char *out, size_t size): out(out), size(size) {}
    void character(char c) { if (used + 1 < size) out[used] = c; ++used; }
    void plain(const char *s) { if (s) while (*s) character(*s++); }
    void text(const char *s) {
        const char hex[] = "0123456789ABCDEF";
        character('"');
        if (s) for (; *s; ++s) {
            const unsigned char c = static_cast<unsigned char>(*s);
            if (c == '"') { character('"'); character('"'); }
            else if (c == '\\') { character('\\'); character('\\'); }
            else if (c < 32 || c == 127) {
                plain("\\x"); character(hex[c >> 4]); character(hex[c & 15]);
            } else character(*s);
        }
        character('"');
    }
    void number(unsigned long n) { char b[24]; snprintf(b,sizeof(b),"%lu",n); plain(b); }
    void signedNumber(int n) { char b[24]; snprintf(b,sizeof(b),"%d",n); plain(b); }
    void separator() { character(','); }
    size_t finish() {
        if (used >= size) { if (size) out[0] = '\0'; return 0; }
        out[used] = '\0'; return used;
    }
private:
    char *out; size_t size; size_t used = 0;
};
const char *confidence(Confidence c) {
    switch(c) { case Confidence::High:return "HIGH"; case Confidence::Medium:return "MEDIUM";
        case Confidence::Low:return "LOW"; } return "UNKNOWN";
}
const char *source(SourceKind s) {
    switch(s) { case SourceKind::Wifi:return "WIFI"; case SourceKind::Ble:return "BLE";
        case SourceKind::Ieee802154:return "IEEE802154"; default:return "UNKNOWN"; }
}
const char *band(Band b) {
    switch(b) { case Band::Band2_4GHz:return "2.4GHz"; case Band::Band5GHz:return "5GHz";
        default:return "UNKNOWN"; }
}
}
size_t format(const Row &r, char *out, size_t size) {
    if (!out || !size) return 0;
    out[0] = '\0';
    if (!r.producer || !*r.producer || !r.bootId || !*r.bootId) return 0;
    Writer w(out,size);
    w.number(1);w.separator();w.text(r.producer);w.separator();w.text(r.bootId);w.separator();
    w.text(r.repeat ? "repeat" : "event");w.separator();w.text(r.recordedLocalTime);w.separator();
    w.number(r.event.eventId);w.separator();w.number(r.event.lastSeen.uptimeMs);w.separator();
    if(r.event.lastSeen.wallClockValid) w.number(r.event.lastSeen.unixSeconds);
    w.separator();w.text(r.detectorName);w.separator();w.text(confidence(r.event.confidence));w.separator();
    w.text(source(r.event.sourceKind));w.separator();w.text(r.event.sourceId);w.separator();
    w.text(r.event.detail);w.separator();w.signedNumber(r.event.rssi);w.separator();
    w.number(r.event.channel);w.separator();w.text(band(r.event.band));w.separator();w.number(r.event.count);
    return w.finish();
}
} }
