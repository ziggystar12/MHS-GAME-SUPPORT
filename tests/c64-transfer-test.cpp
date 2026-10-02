// SPDX-License-Identifier: MIT
// Execute the published transfer include with a simulated C64 bus.
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "../interfaces/teensy/VMC64Transfer.h"

#define FLASHMEM
#define FeatVMVideoDMA
#define Fab04_FullDMACapable
enum { DMA_S_DisableReady, DMA_S_Active };
static int DMA_State;
static bool videoDmaEnabled = true, quietRequested, videoBorderWaiting, videoBorderGrant;
static uint8_t videoTiming = 0x80;
static uint32_t videoBorderCycles, ARM_DWT_CYCCNT, clockUs;
static constexpr uint32_t F_CPU_ACTUAL = 600000000;
static int nS_DMASetup, nS_MaxAdj, nS_DMADataSetup, nS_DMADataHold;
static constexpr int Def_nS_DMASetupNTSC=1, Def_nS_DMASetupPAL=2,
    Def_nS_MaxAdjNTSC=3, Def_nS_MaxAdjPAL=4, Def_nS_DMADataSetupNTSC=5,
    Def_nS_DMADataSetupPAL=6, Def_nS_DMADataHoldNTSC=7, Def_nS_DMADataHoldPAL=8;
static struct { int phase; } indexedVideo;
static uint8_t ram[65536];
static unsigned opened, closed, written;
static bool failPayload;
static uint32_t micros() { return clockUs; }
static void delayMicroseconds(uint32_t n) { clockUs += n; ARM_DWT_CYCCNT += n * 600; }
static bool videoSourceRange(const void *p, uint32_t n) { return p && n; }
static bool PerformDMA(bool read, uint16_t address, uint8_t *out, uint32_t n, bool) {
    assert(read && DMA_State == DMA_S_DisableReady);
    assert(uint32_t(address)+n <= sizeof ram);
    DMA_State = DMA_S_Active; ++opened;
    std::memcpy(out, ram+address, n); return true;
}
static bool AGIContinueDMA(bool read, uint16_t address, uint8_t *data, uint32_t n, bool) {
    assert(!read && DMA_State == DMA_S_Active);
    assert(uint32_t(address)+n <= sizeof ram);
    if (failPayload && n > 1) return false;
    std::memcpy(ram+address, data, n); ++written; return true;
}
static bool CloseDMA() { assert(DMA_State == DMA_S_Active); DMA_State=DMA_S_DisableReady; ++closed; return true; }
static void AGIDMAEmergencyRelease() { DMA_State=DMA_S_DisableReady; }
#include "../integration/teensy/VMC64TransferHost.h"

static uint8_t payload[] = {11, 22, 33, 44};
static VmC64Span spans[] = {{0x6000, 0, 2}, {0x8000, 2, 2}};
static VmC64Transfer request() {
    return {sizeof(VmC64Transfer), VM_C64_TRANSFER_LAST, spans, payload, 2, 4,
        0x0400, 0x0401, 7, 1, 0};
}
static void setup() {
    resetC64Transfer(); std::memset(ram, 0, sizeof ram);
    ram[0x0400]=7; ram[0x0402]=1; clockUs=ARM_DWT_CYCCNT=0;
    videoTiming=0x80; videoDmaEnabled=true; quietRequested=false;
    indexedVideo.phase=0; opened=closed=written=0; failPayload=false;
    assert(DMA_State==DMA_S_DisableReady);
}
int main() {
    setup(); auto r=request();
    assert(submitC64Transfer(&r)==VmVideoResult::Busy);
    pollC64Transfer(); assert(submitC64Transfer(&r)==VmVideoResult::Transferred);
    assert(ram[0x6000]==11 && ram[0x6001]==22 && ram[0x8000]==33 && ram[0x8001]==44);
    assert(ram[0x0401]==1 && opened==closed && DMA_State==DMA_S_DisableReady);

    setup(); r=request(); r.flags=0;
    assert(submitC64Transfer(&r)==VmVideoResult::Busy); pollC64Transfer();
    assert(submitC64Transfer(&r)==VmVideoResult::Transferred && ram[0x0401]==0);
    r.flags=VM_C64_TRANSFER_LAST;
    assert(submitC64Transfer(&r)==VmVideoResult::Busy); pollC64Transfer();
    assert(submitC64Transfer(&r)==VmVideoResult::Transferred && ram[0x0401]==1);

    setup(); r=request(); r.flags=VM_C64_TRANSFER_BORDER;
    videoBorderGrant=true; assert(submitC64Transfer(&r)==VmVideoResult::Busy);
    assert(!videoBorderGrant); pollC64Transfer(); assert(opened==0);
    videoBorderGrant=true; videoBorderCycles=0; ARM_DWT_CYCCNT=300*600;
    pollC64Transfer(); assert(opened==0 && c64TransferResult==VmVideoResult::Busy);
    videoBorderGrant=true; videoBorderCycles=ARM_DWT_CYCCNT;
    pollC64Transfer(); assert(submitC64Transfer(&r)==VmVideoResult::Transferred && ram[0x0401]==1);

    setup(); r=request(); r.flags=VM_C64_TRANSFER_BORDER; ram[0x0400]=0;
    assert(submitC64Transfer(&r)==VmVideoResult::Busy);
    videoBorderGrant=true; videoBorderCycles=0; pollC64Transfer();
    assert(c64TransferResult==VmVideoResult::Busy && opened==closed && DMA_State==DMA_S_DisableReady);
    ram[0x0400]=7; videoBorderGrant=true; videoBorderCycles=ARM_DWT_CYCCNT;
    pollC64Transfer(); assert(submitC64Transfer(&r)==VmVideoResult::Transferred);

    setup(); r=request(); assert(submitC64Transfer(&r)==VmVideoResult::Busy);
    auto other=request(); assert(submitC64Transfer(&other)==VmVideoResult::Failed);
    pollC64Transfer(); assert(submitC64Transfer(&r)==VmVideoResult::Transferred);

    setup(); r=request(); quietRequested=true;
    assert(submitC64Transfer(&r)==VmVideoResult::Busy); pollC64Transfer(); assert(opened==0);
    clockUs=5000000; pollC64Transfer(); assert(submitC64Transfer(&r)==VmVideoResult::Failed && opened==0);

    setup(); r=request(); failPayload=true;
    assert(submitC64Transfer(&r)==VmVideoResult::Busy); pollC64Transfer();
    assert(submitC64Transfer(&r)==VmVideoResult::Failed && ram[0x0401]==2 && opened==closed);

    setup(); r=request(); videoDmaEnabled=false;
    assert(submitC64Transfer(&r)==VmVideoResult::Unavailable && opened==0);
    setup(); r=request(); spans[0].address=0xd000;
    assert(submitC64Transfer(&r)==VmVideoResult::Failed && opened==0); spans[0].address=0x6000;
    r=request(); r.ready=0x0200; r.done=0x0201;
    assert(submitC64Transfer(&r)==VmVideoResult::Failed && opened==0);
    r=request(); spans[0].address=0x0400;
    assert(submitC64Transfer(&r)==VmVideoResult::Failed && opened==0); spans[0].address=0x6000;
    r=request(); r.count=65;
    assert(submitC64Transfer(&r)==VmVideoResult::Failed && opened==0);
    r=request(); r.flags=4;
    assert(submitC64Transfer(&r)==VmVideoResult::Failed && opened==0);

    uint8_t large[2000]{}; VmC64Span largeSpan{0x6000,0,2000};
    r=request(); r.spans=&largeSpan; r.payload=large; r.count=1; r.payload_bytes=2000;
    assert(vm_valid_c64_transfer(r,false,videoSourceRange));
    assert(!vm_valid_c64_transfer(r,true,videoSourceRange));
    std::cout << "C64 transfer: completion, cold upload, fresh grants, retry, timeout, errors and PAL/NTSC bounds passed\n";
}
