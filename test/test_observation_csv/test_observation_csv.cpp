#include "check.h"
#include "core/logic/ObservationCsv.h"
#include <cstring>
using namespace layertime;
using namespace layertime::observation_log;
Row sample() {
    Row r; r.producer="tultra";r.bootId="0011223344556677";
    r.recordedLocalTime="2026-11-13 18:00:00";r.detectorName="DEAUTH";
    r.event.eventId=7;r.event.lastSeen.uptimeMs=1234;r.event.sourceKind=SourceKind::Wifi;
    r.event.band=Band::Band2_4GHz;r.event.confidence=Confidence::High;
    strcpy(r.event.sourceId,"AA:BB:CC:DD:EE:FF");strcpy(r.event.detail,"observed request");
    r.event.rssi=-50;r.event.channel=6;r.event.count=17;return r;
}
void identity_counts_and_unknown_wall_clock_survive() {
    char out[kRowBufferSize];auto r=sample();
    CHECK_TRUE(format(r,out,sizeof(out))>0);
    CHECK_STR("1,\"tultra\",\"0011223344556677\",\"event\",\"2026-11-13 18:00:00\",7,1234,,\"DEAUTH\",\"HIGH\",\"WIFI\",\"AA:BB:CC:DD:EE:FF\",\"observed request\",-50,6,\"2.4GHz\",17",out);
    r.repeat=true;r.event.lastSeen.wallClockValid=true;r.event.lastSeen.unixSeconds=1700000000;
    format(r,out,sizeof(out));CHECK_TRUE(strstr(out,"\"repeat\"")!=nullptr);
    CHECK_TRUE(strstr(out,",1234,1700000000,")!=nullptr);
}
void untrusted_names_cannot_create_rows_or_columns() {
    auto r=sample();strcpy(r.event.detail,"x,\"y\"\n\\x0A\t");
    char out[kRowBufferSize];CHECK_TRUE(format(r,out,sizeof(out))>0);
    CHECK_TRUE(strstr(out,"\"x,\"\"y\"\"\\x0A\\\\x0A\\x09\"")!=nullptr);
    CHECK_TRUE(strchr(out,'\n')==nullptr);CHECK_TRUE(strchr(out,'\t')==nullptr);
}
void short_buffers_never_expose_a_partial_row() {
    auto r=sample();char full[kRowBufferSize];const size_t n=format(r,full,sizeof(full));
    for(size_t size=1;size<=n;++size){char out[kRowBufferSize];memset(out,'X',sizeof(out));
        CHECK_INT(0,format(r,out,size));CHECK_INT(0,out[0]);CHECK_CHAR('X',out[size]);}
    char exact[kRowBufferSize];CHECK_INT(n,format(r,exact,n+1));CHECK_STR(full,exact);
    CHECK_INT(0,format(r,nullptr,1));CHECK_INT(0,format(r,exact,0));
}
void missing_boot_identity_is_not_an_observation() {
    auto r=sample();r.bootId="";char out[kRowBufferSize];CHECK_INT(0,format(r,out,sizeof(out)));CHECK_INT(0,out[0]);
}
void maximum_observed_text_and_counts_fit() {
    auto r=sample();memset(r.event.detail,'\x01',sizeof(r.event.detail)-1);
    r.event.detail[sizeof(r.event.detail)-1]=0;r.event.eventId=0xffffffff;r.event.count=0xffffffff;
    char out[kRowBufferSize];CHECK_TRUE(format(r,out,sizeof(out))>0);
    CHECK_TRUE(strstr(out,",4294967295")!=nullptr);
}
int main(int argc,char**argv){CHECK_MAIN(argc,argv);CASE(identity_counts_and_unknown_wall_clock_survive);
CASE(untrusted_names_cannot_create_rows_or_columns);CASE(short_buffers_never_expose_a_partial_row);
CASE(missing_boot_identity_is_not_an_observation);CASE(maximum_observed_text_and_counts_fit);CHECK_SUMMARY();}
