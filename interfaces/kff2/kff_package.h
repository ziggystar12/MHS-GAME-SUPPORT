/* SPDX-License-Identifier: MIT
 * KFP1: KFF2 single-file MPE package. All integers are little endian.
 * The reader is shared by firmware and host integration tests. */
#ifndef KFF_PACKAGE_H
#define KFF_PACKAGE_H
#include "kff_mpe.h"
#include "kff_lz4.h"
#if defined(__arm__)
#define MPM_COLD __attribute__((section(".flash"),noinline))
#define KFP_COLD __attribute__((section(".flash"),noinline))
#else
#define KFP_COLD inline
#endif
#include "mgc1_directory.h"
#include <string.h>
#define KFP_MAGIC 0x3150464bu
#define KFP_PLATFORM 0x3246464bu
#define KFP_BLOCK 4096u
#define KFP_CACHE_BLOCKS 12u
#define KFP_LIMIT (64u*1024u*1024u)
typedef struct {
    uint32_t magic,version,header_bytes,file_bytes,platform;
    uint32_t client_offset,client_bytes,client_crc;
    uint32_t module_offset,module_bytes,module_crc;
    uint32_t game_offset,game_bytes,game_crc,block_bytes,header_crc;
    char game_id[16];
    uint32_t reserved[12];
} KfpHeader;
typedef int (*KfpRead)(void *,uint32_t,void *,uint32_t);
typedef struct {
    KfpHeader header;
    KfpRead read;
    void *context;
    uint32_t cached_block[KFP_CACHE_BLOCKS];
    uint8_t age[KFP_CACHE_BLOCKS];
    uint8_t cache[KFP_CACHE_BLOCKS][KFP_BLOCK];
    uint8_t packed[KFP_BLOCK]; /* Fixed workspace, never on the game stack. */
} KfpReader;
static inline int kfp_game_id_valid(const char *id){
    static const char ids[][16]={"MONKEY","LOOM","DOOM","SQ3","LSL2","PQ2","COLONEL","HAMSTEROS","MULEPLUS","NESVM","GBVM","GGVM","AGIVM","KYRANDIA","ANOTHER","MM4"};
    for(unsigned i=0;i<sizeof ids/sizeof ids[0];i++)if(!memcmp(id,ids[i],16))return 1;return 0;
}
static inline int kfp_module_matches(const KfpHeader *package,const KfmHeader *module){
    return package&&module&&((!memcmp(package->game_id,"SQ3",4)||!memcmp(package->game_id,"LSL2",5)||!memcmp(package->game_id,"PQ2",4)||!memcmp(package->game_id,"COLONEL",8))?module->reserved[0]==0:
        !memcmp(package->game_id,"DOOM",5)?module->reserved[0]==KFM_DOOM_PROFILE:
        (!memcmp(package->game_id,"MONKEY",7)||!memcmp(package->game_id,"LOOM",5)||!memcmp(package->game_id,"HAMSTEROS",10)||!memcmp(package->game_id,"MULEPLUS",9)||!memcmp(package->game_id,"NESVM",6)||!memcmp(package->game_id,"GBVM",5)||!memcmp(package->game_id,"GGVM",5)||!memcmp(package->game_id,"AGIVM",6)||!memcmp(package->game_id,"KYRANDIA",9)||!memcmp(package->game_id,"ANOTHER",8)||!memcmp(package->game_id,"MM4",4))&&module->reserved[0]==0);
}
static inline void kfp_cache_clear(KfpReader *r){for(unsigned i=0;i<KFP_CACHE_BLOCKS;i++){r->cached_block[i]=~0u;r->age[i]=0;}}
static inline unsigned kfp_cache_slot(const KfpReader *r,uint32_t block){for(unsigned i=0;i<KFP_CACHE_BLOCKS;i++)if(r->cached_block[i]==block)return i;return KFP_CACHE_BLOCKS;}
static inline void kfp_cache_touch(KfpReader *r,unsigned slot){
    unsigned previous=r->cached_block[slot]==~0u?255:r->age[slot];
    for(unsigned i=0;i<KFP_CACHE_BLOCKS;i++)if(i!=slot&&r->cached_block[i]!=~0u&&r->age[i]<previous)r->age[i]++;
    r->age[slot]=0;
}
static inline int kfp_header_valid(const KfpHeader *h,uint32_t bytes){
    if(sizeof(*h)!=128||h->magic!=KFP_MAGIC||(h->version!=1&&h->version!=2)||h->header_bytes!=128||
       h->platform!=KFP_PLATFORM||bytes>KFP_LIMIT||h->file_bytes!=bytes||
       h->client_offset!=128||h->client_bytes<3||h->client_bytes>0x1800||
       h->module_offset!=128+h->client_bytes||h->module_bytes<64||
       h->module_bytes>64+KFM_DOOM_CODE_SIZE+KFM_DOOM_DATA_SIZE||
       h->game_offset!=h->module_offset+h->module_bytes||
       !h->game_bytes||h->game_bytes>KFP_LIMIT||h->block_bytes!=KFP_BLOCK||
       !kfp_game_id_valid(h->game_id))return 0;
    if(h->version==1){if(h->file_bytes!=h->game_offset+h->game_bytes+4*((h->game_bytes+4095)/4096))return 0;}
    else if(h->game_offset>bytes||16*((h->game_bytes+4095)/4096)>=bytes-h->game_offset)return 0;
    for(unsigned i=h->version==2?1:0;i<12;i++)if(h->reserved[i])return 0;
    KfpHeader check=*h;check.header_crc=0;return kfm_crc(&check,sizeof check)==h->header_crc;
}
static inline int kfp_record_valid(const KfpHeader *h,const uint32_t *entry,uint32_t block){
    uint32_t count=(h->game_bytes+4095)/4096,decoded=h->game_bytes-block*4096;
    if(decoded>4096)decoded=4096;uint32_t stored=entry[1]&0x7fffffffu;
    return block<count&&entry[2]==decoded&&stored&&stored<=decoded&&
        ((entry[1]&0x80000000u)?stored==decoded:stored<decoded)&&
        entry[0]>=h->game_offset+count*16&&entry[0]<=h->file_bytes&&stored<=h->file_bytes-entry[0];
}
static inline int kfp_index_valid(KfpReader *r){
    const KfpHeader *h=&r->header;uint32_t count=(h->game_bytes+4095)/4096,cursor=h->game_offset+count*16,crc=~0u;
    for(uint32_t block=0;block<count;){uint32_t n=count-block;if(n>256)n=256;
        if(!r->read(r->context,h->game_offset+block*16,r->packed,n*16))return 0;
        for(uint32_t i=0;i<n*16;i++){crc^=r->packed[i];for(unsigned bit=0;bit<8;bit++)crc=(crc>>1)^((0u-(crc&1))&0xedb88320u);}
        for(uint32_t i=0;i<n;i++,block++){uint32_t entry[4];memcpy(entry,r->packed+i*16,16);
            if(!kfp_record_valid(h,entry,block)||entry[0]!=cursor)return 0;cursor+=entry[1]&0x7fffffffu;
        }
    }return cursor==h->file_bytes&&~crc==h->reserved[0];
}
/* A portable MPE is an unchanged MGC1 with a KFF2 target directory. The
 * synthesized header is private reader state, never an on-disk KFP revision.
 * Existing engine/client ABIs and filesystem paths are retained. */
static KFP_COLD int kfp_mount_mgc(KfpReader *r,KfpRead read,void *context,uint32_t bytes){
    MpmDirectory d;MpmEntry target,client,module,game;uint8_t m[128];
    if(!mpm_open(&d,read,context,bytes)||
       !mpm_find(&d,"targets/kff2/target.bin",&target)||target.flags||target.bytes!=128||
       !mpm_read(&d,target.offset,m,128)||mpm_crc(m,128)!=target.crc||
       memcmp(m,"MPT1",4)||m[4]!=1||m[5]||m[6]!=128||m[7]||
       mpm_u32(m+8)!=KFP_PLATFORM||mpm_u32(m+12)!=KFM_ABI||
       !mpm_text(m+16,16,0)||!kfp_game_id_valid((const char*)m+16)||
       (strcmp(d.id,(const char*)m+16)&&(strcmp(d.id,"DOOMVM")||strcmp((const char*)m+16,"DOOM"))&&(strcmp(d.id,"SCI0VM")||strcmp((const char*)m+16,"SQ3")))||!mpm_text(m+32,64,0)||
       !mpm_text(m+96,16,0)||!mpm_zero(m+112,12))return 0;
    uint32_t crc=mpm_u32(m+124);memset(m+124,0,4);if(mpm_crc(m,128)!=crc)return 0;
    if(!mpm_find(&d,"targets/kff2/client.prg",&client)||client.flags||client.bytes<3||client.bytes>0x1800||
       !mpm_find(&d,"targets/kff2/engine.kfm",&module)||module.flags||module.bytes<64||module.bytes>64+KFM_DOOM_CODE_SIZE+KFM_DOOM_DATA_SIZE||
       !mpm_name((const char*)m+32)||!mpm_find(&d,(const char*)m+32,&game)||game.flags!=1)return 0;
    /* Target executables are verified before the caller launches its C64
     * client. Game members retain independent CRC-checked 4 KiB blocks. */
    KfmHeader engine;uint8_t load[2];
    if(!mpm_check_crc(&d,client.offset,client.bytes,client.crc)||
       !mpm_check_crc(&d,module.offset,module.bytes,module.crc)||
       !mpm_read(&d,client.offset,load,2)||load[0]!=1||load[1]!=8||
       !mpm_read(&d,module.offset,&engine,sizeof engine)||!kfm_header_valid(&engine,module.bytes))return 0;
    KfpHeader *h=&r->header;memset(h,0,sizeof *h);
    h->magic=KFP_MAGIC;h->version=2;h->header_bytes=128;h->file_bytes=bytes;h->platform=KFP_PLATFORM;
    h->client_offset=client.offset;h->client_bytes=client.bytes;h->client_crc=client.crc;
    h->module_offset=module.offset;h->module_bytes=module.bytes;h->module_crc=module.crc;
    h->game_offset=game.table;h->game_bytes=game.bytes;h->game_crc=game.crc;h->block_bytes=4096;
    memcpy(h->game_id,m+16,16);h->reserved[0]=game.table_crc;
    if(!kfp_module_matches(h,&engine))return 0;
    r->read=read;r->context=context;return 1;
}
static inline int kfp_mount(KfpReader *r,KfpRead read,void *context,uint32_t bytes){
    memset(r,0,sizeof(*r));kfp_cache_clear(r);
    uint8_t magic[4];
    if(!read||bytes<128||!read(context,0,magic,4))return 0;
    if(!memcmp(magic,"C64 ",4))return kfp_mount_mgc(r,read,context,bytes);
    if(!read||!read(context,0,&r->header,sizeof r->header)||!kfp_header_valid(&r->header,bytes))return 0;
    r->read=read;r->context=context;
    if(r->header.version==2&&!kfp_index_valid(r)){r->read=0;return 0;}return 1;
}
/* Verify a complete contiguous member before executing any of its bytes. */
static inline int kfp_verify(KfpReader *r,uint32_t offset,uint32_t bytes,uint32_t expected){
    if(!r->read||offset>r->header.file_bytes||bytes>r->header.file_bytes-offset)return 0;
    uint32_t crc=~0u;kfp_cache_clear(r);
    while(bytes){uint32_t n=bytes>KFP_BLOCK?KFP_BLOCK:bytes;
        if(!r->read(r->context,offset,r->cache[0],n))return 0;
        for(uint32_t i=0;i<n;i++){crc^=r->cache[0][i];for(unsigned b=0;b<8;b++)crc=(crc>>1)^((0u-(crc&1))&0xedb88320u);}
        offset+=n;bytes-=n;
    }return ~crc==expected;
}
/* Each stored game block is a CRC32 followed by up to 4096 raw bytes.
 * No unverified block reaches the engine; partial and crossing reads work. */
static inline int kfp_game_cached(const KfpReader *r,uint32_t offset,uint32_t bytes){
    if(!r->read||offset>r->header.game_bytes||bytes>r->header.game_bytes-offset)return 0;
    if(!bytes)return 1;
    uint32_t first=offset/KFP_BLOCK,last=(offset+bytes-1)/KFP_BLOCK;
    if(last-first>=KFP_CACHE_BLOCKS)return 0;
    for(uint32_t block=first;block<=last;block++)if(kfp_cache_slot(r,block)==KFP_CACHE_BLOCKS)return 0;
    return 1;
}
static inline int kfp_game_read(KfpReader *r,uint32_t offset,void *data,uint32_t bytes){
    if(!r->read||offset>r->header.game_bytes||bytes>r->header.game_bytes-offset)return 0;
    uint8_t *out=(uint8_t*)data;
    while(bytes){uint32_t block=offset/KFP_BLOCK,within=offset%KFP_BLOCK;
        uint32_t decoded=r->header.game_bytes-block*KFP_BLOCK;if(decoded>KFP_BLOCK)decoded=KFP_BLOCK;
        unsigned slot=kfp_cache_slot(r,block);
        if(slot==KFP_CACHE_BLOCKS){uint32_t crc;
            slot=0;for(unsigned i=0;i<KFP_CACHE_BLOCKS;i++){if(r->cached_block[i]==~0u){slot=i;break;}if(r->age[i]>r->age[slot])slot=i;}
            r->cached_block[slot]=~0u;
            if(r->header.version==1){uint32_t physical=r->header.game_offset+block*(KFP_BLOCK+4);
                if(!r->read(r->context,physical,&crc,4)||!r->read(r->context,physical+4,r->cache[slot],decoded))return 0;
            }else{uint32_t entry[4];
                if(!r->read(r->context,r->header.game_offset+block*16,entry,16)||!kfp_record_valid(&r->header,entry,block))return 0;
                uint32_t stored=entry[1]&0x7fffffffu;crc=entry[3];
                if(entry[1]&0x80000000u){if(!r->read(r->context,entry[0],r->cache[slot],decoded))return 0;}
                else if(!r->read(r->context,entry[0],r->packed,stored)||!kfp_lz4(r->packed,stored,r->cache[slot],decoded))return 0;
            }
            if(kfm_crc(r->cache[slot],decoded)!=crc)return 0;
            kfp_cache_touch(r,slot);r->cached_block[slot]=block;
        }else{
            kfp_cache_touch(r,slot);
        }
        uint32_t n=decoded-within;if(n>bytes)n=bytes;
        memcpy(out,r->cache[slot]+within,n);out+=n;offset+=n;bytes-=n;
    }return 1;
}
#endif
