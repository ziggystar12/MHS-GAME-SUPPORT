/* SPDX-License-Identifier: MIT
 * Bounded 4 KiB decoder adapted from the working Teensy VMGameCart reader.
 * Format: https://github.com/lz4/lz4/blob/dev/doc/lz4_Block_format.md */
#ifndef KFF_LZ4_H
#define KFF_LZ4_H
#include <stdint.h>
#include <string.h>
static inline int kfp_lz4(const uint8_t *in,uint32_t input_bytes,uint8_t *out,uint32_t output_bytes){
    if(!output_bytes||output_bytes>4096||!input_bytes||input_bytes>4096)return 0;
    uint32_t ip=0,op=0;
    while(ip<input_bytes){const uint8_t token=in[ip++];uint32_t literals=token>>4;
        if(literals==15){uint8_t b;do{if(ip==input_bytes)return 0;b=in[ip++];literals+=b;if(literals>output_bytes)return 0;}while(b==255);}
        if(literals>input_bytes-ip||literals>output_bytes-op)return 0;
        memcpy(out+op,in+ip,literals);ip+=literals;op+=literals;
        if(ip==input_bytes)return op==output_bytes;
        if(input_bytes-ip<2)return 0;uint32_t distance=in[ip]|((uint32_t)in[ip+1]<<8);ip+=2;
        if(!distance||distance>op)return 0;uint32_t match=token&15;
        if(match==15){uint8_t b;do{if(ip==input_bytes)return 0;b=in[ip++];match+=b;if(match>output_bytes)return 0;}while(b==255);}
        match+=4;if(match>output_bytes-op)return 0;
        while(match--){out[op]=out[op-distance];++op;}
    }return 0;
}
#endif
