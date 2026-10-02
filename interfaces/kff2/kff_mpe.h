/* SPDX-License-Identifier: MIT
 * MHS KFF2 prototype ABI. Separate from the Teensy module image format. */
#ifndef KFF_MPE_H
#define KFF_MPE_H
#include <stdint.h>
#include <stddef.h>
#define KFM_MAGIC 0x314d464bu
#define KFM_ABI 8u
#define KFM_PROTOTYPE 0x20u /* BCD, displayed as two hex digits by the client. */
#define KFM_CODE_BASE 0x24000000u
#define KFM_CODE_SIZE 0x20000u
#define KFM_DATA_BASE 0x24020000u
#define KFM_DATA_SIZE 0x60000u
#define KFM_GUEST_BASE 0x24080000u
#define KFM_GUEST_SIZE 0x80000u
/* Doom profile 1 keeps its proven 416 KiB zone and gives its renderer and
 * read-only tables more room. Both profiles end at the same AXI boundary. */
#define KFM_DOOM_PROFILE 1u
#define KFM_DOOM_CODE_SIZE 0x28000u
#define KFM_DOOM_DATA_BASE 0x24028000u
#define KFM_DOOM_DATA_SIZE 0x70000u
#define KFM_DOOM_GUEST_BASE 0x24098000u
#define KFM_DOOM_GUEST_SIZE 0x68000u
#define KFM_FULL_FRAME_BYTES (13u+20296u+4096u)
#define KFM_CHUNKS 241u
#define KFM_FRAME_BYTES (KFM_FULL_FRAME_BYTES+3u*KFM_CHUNKS+1u)
#define KFM_DELTA_PACKETS 1
#define KFM_HEADER_BYTES 64u
typedef struct {
    uint32_t magic, abi, header_bytes, code_bytes, data_bytes, bss_bytes;
    uint32_t entry, code_base, data_base, payload_crc, header_crc;
    uint32_t reserved[5];
} KfmHeader;
typedef struct { uint8_t buttons, display, overflow, protocol; } KfmInput;
typedef struct {
    uint32_t abi, bytes;
    uint32_t (*micros_now)(void);
    uint32_t (*open)(const char *,uint32_t,uint32_t *,uint32_t *);
    int32_t (*read)(uint32_t,uint32_t,void *,uint32_t);
    int32_t (*write)(uint32_t,uint32_t,const void *,uint32_t);
    void (*close)(uint32_t);
    int32_t (*file_op)(uint32_t,uint32_t,uint32_t,const char *,const char *);
    uint32_t (*input)(KfmInput *);
    uint32_t (*video_busy)(void);
    uint32_t (*video_submit)(const uint8_t *,uint32_t);
    void (*sid)(const uint8_t *);
    void (*fail)(uint32_t,uint32_t);
    uint32_t (*video_standard)(void); /* 0 PAL, 1 NTSC */
    void (*loading)(uint32_t,uint32_t,uint32_t,uint32_t); /* stage, %, x, y */
    void (*metric)(uint32_t,uint32_t); /* game/Prism/file work, microseconds */
    uint32_t (*diagnostic)(void); /* Local diagnostic request; pause when DMA/storage idle */
} KfmPlatform;
static inline uint32_t kfm_crc(const void *data,uint32_t bytes) {
    const uint8_t *p=(const uint8_t *)data;uint32_t c=~0u;
    while(bytes--){c^=*p++;for(unsigned b=0;b<8;b++)c=(c>>1)^((0u-(c&1))&0xedb88320u);}return ~c;
}
static inline int kfm_header_valid(const KfmHeader *h,uint32_t bytes) {
    const uint32_t profile=h->reserved[0];
    if(profile>KFM_DOOM_PROFILE)return 0;
    const uint32_t code_size=profile?KFM_DOOM_CODE_SIZE:KFM_CODE_SIZE;
    const uint32_t data_base=profile?KFM_DOOM_DATA_BASE:KFM_DATA_BASE;
    const uint32_t data_size=profile?KFM_DOOM_DATA_SIZE:KFM_DATA_SIZE;
    if(h->magic!=KFM_MAGIC||h->abi!=KFM_ABI||h->header_bytes!=64||
       h->code_base!=KFM_CODE_BASE||h->data_base!=data_base||
       !h->code_bytes||h->code_bytes>code_size||
       h->data_bytes>data_size||h->bss_bytes>data_size-h->data_bytes||
       !(h->entry&1)||(h->entry&~1u)<KFM_CODE_BASE||
       (h->entry&~1u)>=KFM_CODE_BASE+h->code_bytes||
       bytes!=64+h->code_bytes+h->data_bytes)return 0;
    for(unsigned i=1;i<5;i++)if(h->reserved[i])return 0;
    KfmHeader check=*h;check.header_crc=0;return kfm_crc(&check,sizeof check)==h->header_crc;
}
#endif
