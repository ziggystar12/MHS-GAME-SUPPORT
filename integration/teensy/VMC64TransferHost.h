// SPDX-License-Identifier: MIT
// Included in VmRuntime after the video grant mailbox has been declared.
static const VmC64Transfer* c64Transfer;
static VmVideoResult c64TransferResult=VmVideoResult::Unavailable;
static uint32_t c64TransferStarted;
// Published only if the module reports a fatal error in the same pump as a
// transport failure. No ABI or request layout change, and a handled failure
// cannot replace an unrelated error on a later foreground turn.
static uint32_t c64TransferFaultDetail;
static uint8_t c64TransferWaitReason,c64TransferReady,c64TransferGrants;
enum : uint8_t { C64BadRequest=1,C64RequestConflict,C64NoGrant,C64StaleGrant,
    C64NotReady,C64OpenFailed,C64Capability,C64PayloadFailed,C64DoneFailed,C64CloseFailed };
static uint32_t c64TransferFault(){return c64TransferFaultDetail;}
static void noteC64TransferFault(uint8_t reason){c64TransferFaultDetail=reason|uint32_t(c64TransferReady)<<8|uint32_t(c64TransferGrants)<<16;}
static void failC64Transfer(uint8_t reason){
    noteC64TransferFault(reason);c64TransferResult=VmVideoResult::Failed;videoBorderWaiting=videoBorderGrant=false;
}
static uint32_t c64Timing(){return videoDmaEnabled?videoTiming:0;}
static void resetC64Transfer(){c64Transfer=nullptr;c64TransferResult=VmVideoResult::Unavailable;videoBorderWaiting=videoBorderGrant=false;c64TransferFaultDetail=0;c64TransferReady=0xff;c64TransferGrants=0;}
static FLASHMEM void pollC64Transfer(){
#if defined(FeatVMVideoDMA) && defined(Fab04_FullDMACapable)
    if(!c64Transfer||c64TransferResult!=VmVideoResult::Busy)return;
    const auto& r=*c64Transfer;const bool sliced=r.flags&VM_C64_TRANSFER_BORDER;
    if(uint32_t(micros()-c64TransferStarted)>=5000000u){failC64Transfer(c64TransferWaitReason);return;}
    if(DMA_State!=DMA_S_DisableReady||quietRequested)return;
    uint32_t grant=0;const uint32_t cyclesPerUs=F_CPU_ACTUAL/1000000u;
    if(sliced){
        if(!videoBorderGrant)return;
        grant=videoBorderCycles;videoBorderGrant=false;
        if(c64TransferGrants!=255)++c64TransferGrants;
        if(uint32_t(ARM_DWT_CYCCNT-grant)>cyclesPerUs*250u){c64TransferWaitReason=C64StaleGrant;return;}
    }
    const bool ntsc=videoTiming&1;
    nS_DMASetup=ntsc?Def_nS_DMASetupNTSC:Def_nS_DMASetupPAL;
    nS_MaxAdj=ntsc?Def_nS_MaxAdjNTSC:Def_nS_MaxAdjPAL;
    nS_DMADataSetup=ntsc?Def_nS_DMADataSetupNTSC:Def_nS_DMADataSetupPAL;
    nS_DMADataHold=ntsc?Def_nS_DMADataHoldNTSC:Def_nS_DMADataHoldPAL;
    uint8_t mailbox[3]{},fault=C64OpenFailed;bool okay;
    for(;;){
        fault=C64OpenFailed;
        okay=PerformDMA(true,r.ready,mailbox,sliced?1:3,false);
        if(!okay)break;
        c64TransferReady=mailbox[0];
        if(mailbox[0]==r.ready_value&&(!sliced||uint32_t(ARM_DWT_CYCCNT-grant)<=cyclesPerUs*350u))break;
        c64TransferWaitReason=mailbox[0]!=r.ready_value?C64NotReady:C64StaleGrant;
        okay=CloseDMA();
        if(!okay){fault=C64CloseFailed;break;}
        if(!sliced){
            if(uint32_t(micros()-c64TransferStarted)<250000u)return;
            okay=false;fault=C64NotReady;break;
        }
        // The C64 announces the grant before publishing Ready. Returning to
        // desktop/filesystem work here can lose every short readiness window.
        // Release the bus for its remaining instructions, then retry within
        // this SAME grant's original deadline. Never extend the grant.
        if(uint32_t(ARM_DWT_CYCCNT-grant)>cyclesPerUs*234u)return;
        delayMicroseconds(16);
        if(uint32_t(ARM_DWT_CYCCNT-grant)>cyclesPerUs*250u)return;
    }
    if(okay){
        if(!sliced&&mailbox[2]!=r.capability){okay=false;fault=C64Capability;}
        for(unsigned i=0;i<r.count&&okay;++i){const auto& s=r.spans[i];okay=AGIContinueDMA(false,s.address,const_cast<uint8_t*>(r.payload)+s.offset,s.count,false);if(!okay)fault=C64PayloadFailed;}
        // Cold uploads may take multiple bounded requests while the receiver
        // stays parked. A border slice always resumes the running receiver.
        mailbox[0]=okay?uint8_t(sliced||(r.flags&VM_C64_TRANSFER_LAST)?1:0):2;
        if(DMA_State!=DMA_S_DisableReady){const bool done=AGIContinueDMA(false,r.done,mailbox,1,false);if(okay&&!done)fault=C64DoneFailed;okay=done&&okay;}
        const bool closed=CloseDMA();if(okay&&!closed)fault=C64CloseFailed;okay=closed&&okay;
        if(okay){c64TransferResult=VmVideoResult::Transferred;videoBorderWaiting=videoBorderGrant=false;return;}
    }
    if(DMA_State!=DMA_S_DisableReady)AGIDMAEmergencyRelease();
    failC64Transfer(fault);
#endif
}
static FLASHMEM VmVideoResult submitC64Transfer(const VmC64Transfer* request){
#if defined(FeatVMVideoDMA) && defined(Fab04_FullDMACapable)
    if(!videoDmaEnabled||(videoTiming&0xfe)!=0x80)return VmVideoResult::Unavailable;
    if(c64Transfer){
        if(c64Transfer!=request){noteC64TransferFault(C64RequestConflict);return VmVideoResult::Failed;}
        const auto result=c64TransferResult;
        if(result!=VmVideoResult::Busy)c64Transfer=nullptr;
        return result;
    }
    if(!request||!videoSourceRange(request,sizeof *request)||indexedVideo.phase||
       !vm_valid_c64_transfer(*request,(videoTiming&1)!=0,videoSourceRange)){noteC64TransferFault(C64BadRequest);return VmVideoResult::Failed;}
    c64Transfer=request;c64TransferResult=VmVideoResult::Busy;c64TransferStarted=micros();
    c64TransferFaultDetail=0;c64TransferWaitReason=(request->flags&VM_C64_TRANSFER_BORDER)?C64NoGrant:C64NotReady;c64TransferReady=0xff;c64TransferGrants=0;
    // Preparation happened before this call; never reuse its stale grant.
    videoBorderGrant=false;videoBorderWaiting=(request->flags&VM_C64_TRANSFER_BORDER)!=0;
    return VmVideoResult::Busy;
#else
    return VmVideoResult::Unavailable;
#endif
}
