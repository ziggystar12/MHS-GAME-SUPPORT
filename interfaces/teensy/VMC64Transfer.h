// SPDX-License-Identifier: MIT
#pragma once
#include "VMABI.h"

// Policy-free C64 RAM transport. A package owns its pixels, raster and receiver.
// Validate the entire immutable request before opening the bus. No I/O writes,
// address wrap, overlapping mailbox or unbounded scatter list is admitted.
template<class Range> static bool vm_valid_c64_transfer(const VmC64Transfer& r,bool ntsc,Range range){
    if(r.bytes!=sizeof r||(r.flags&~3u)||!r.count||r.count>64||!r.payload_bytes||r.payload_bytes>2048||
       !range(r.spans,r.count*sizeof(VmC64Span))||!range(r.payload,r.payload_bytes)||
       r.ready<0x300||r.ready>0xcffd||r.done!=r.ready+1||!r.ready_value||r.reserved)return false;
    unsigned cost=0,used=0;
    for(unsigned i=0;i<r.count;++i){const auto& s=r.spans[i];const unsigned end=unsigned(s.address)+s.count;
        if(!s.count||s.offset!=used||s.count>r.payload_bytes-used||s.address<0x300||end>0x10000||
           (s.address<0xe000&&end>0xd000)||(s.address<r.ready+3u&&end>r.ready))return false;
        used+=s.count;cost+=s.count+16;
    }
    return used==r.payload_bytes&&cost<=unsigned(ntsc?1536:2048);
}
