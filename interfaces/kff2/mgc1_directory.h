/* SPDX-License-Identifier: MIT
 * Bounded, allocation-free MGC1 directory admission for device adapters.
 * This does not change MGC1 or the existing Teensy reader. */
#ifndef MPE_MGC1_DIRECTORY_H
#define MPE_MGC1_DIRECTORY_H
#include <stdint.h>
#include <string.h>
#ifndef MPM_COLD
#define MPM_COLD inline
#endif
#define MPM_PREFIX 0x6070u
#define MPM_LIMIT (64u*1024u*1024u)
typedef int (*MpmRead)(void *,uint32_t,void *,uint32_t);
typedef struct {
    char name[64];
    uint32_t flags,bytes,offset,stored,crc,blocks,table,table_crc;
} MpmEntry;
typedef struct {
    MpmRead read;void *context;
    uint32_t bytes,count,module,content_index;
    char id[24],content[64];
} MpmDirectory;
static MPM_COLD uint32_t mpm_u32(const uint8_t *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static MPM_COLD uint32_t mpm_crc_step(uint32_t c,const uint8_t *p,uint32_t n){while(n--){c^=*p++;for(unsigned b=0;b<8;b++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);}return c;}
static MPM_COLD uint32_t mpm_crc(const void *p,uint32_t n){return ~mpm_crc_step(~0u,(const uint8_t*)p,n);}
static MPM_COLD int mpm_range(uint32_t at,uint32_t n,uint32_t total){return at<=total&&n<=total-at;}
static MPM_COLD int mpm_read(const MpmDirectory *d,uint32_t at,void *out,uint32_t n){return d->read&&mpm_range(at,n,d->bytes)&&d->read(d->context,at,out,n);}
static MPM_COLD int mpm_zero(const uint8_t *p,uint32_t n){while(n--)if(*p++)return 0;return 1;}
static MPM_COLD int mpm_text(const uint8_t *p,uint32_t n,char *out){int ended=0;for(uint32_t i=0;i<n;i++){uint8_t c=p[i];if(!c)ended=1;else if(ended||c<32||c>126)return 0;if(out)out[i]=(char)c;}return ended;}
static MPM_COLD char mpm_upper(char c){return c>='a'&&c<='z'?(char)(c-32):c;}
static MPM_COLD int mpm_compare(const char *a,const char *b){while(*a&&mpm_upper(*a)==mpm_upper(*b)){a++;b++;}return (uint8_t)mpm_upper(*a)-(uint8_t)mpm_upper(*b);}
static MPM_COLD int mpm_name(const char *p){
    unsigned n=0;int first=1;
    while(*p){char c=*p++;if(++n>=64)return 0;if(c=='/'){if(first)return 0;first=1;continue;}
        int alnum=(c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9');
        if((first&&!alnum)||(!alnum&&c!='_'&&c!='-'&&c!='.')||(c=='.'&&*p=='.'))return 0;first=0;
    }return n&&!first;
}
static MPM_COLD int mpm_check_crc(const MpmDirectory *d,uint32_t at,uint32_t n,uint32_t expected){
    uint8_t b[256];uint32_t c=~0u;
    if(!mpm_range(at,n,d->bytes))return 0;
    while(n){uint32_t take=n>sizeof b?sizeof b:n;if(!mpm_read(d,at,b,take))return 0;c=mpm_crc_step(c,b,take);at+=take;n-=take;}
    return ~c==expected;
}
static MPM_COLD int mpm_entry(const MpmDirectory *d,uint32_t i,MpmEntry *out){
    uint8_t b[128];if(i>=d->count||!mpm_read(d,MPM_PREFIX+i*128,b,sizeof b)||!mpm_text(b,64,out->name)||!mpm_name(out->name)||!mpm_zero(b+96,32))return 0;
    if(!mpm_compare(out->name,"SAVES")||(!strncmp(out->name,"SAVES/",6)))return 0;
    const char *s=out->name;if(mpm_upper(s[0])=='S'&&mpm_upper(s[1])=='A'&&mpm_upper(s[2])=='V'&&mpm_upper(s[3])=='E'&&mpm_upper(s[4])=='S'&&(!s[5]||s[5]=='/'))return 0;
    out->flags=mpm_u32(b+64);out->bytes=mpm_u32(b+68);out->offset=mpm_u32(b+72);out->stored=mpm_u32(b+76);out->crc=mpm_u32(b+80);
    out->blocks=mpm_u32(b+84);out->table=mpm_u32(b+88);out->table_crc=mpm_u32(b+92);return 1;
}
static MPM_COLD int mpm_find(const MpmDirectory *d,const char *name,MpmEntry *out){
    for(uint32_t i=0;i<d->count;i++){MpmEntry e;if(!mpm_entry(d,i,&e))return 0;if(!mpm_compare(e.name,name)){*out=e;return 1;}}return 0;
}
static MPM_COLD int mpm_open(MpmDirectory *d,MpmRead read,void *context,uint32_t bytes){
    uint8_t h[256];MpmEntry previous,e;uint32_t cursor,expected;
    memset(d,0,sizeof *d);if(!read||bytes<MPM_PREFIX||bytes>MPM_LIMIT)return 0;d->read=read;d->context=context;d->bytes=bytes;
    if(!mpm_read(d,0,h,64)||memcmp(h,"C64 CARTRIDGE   ",16)||mpm_u32(h+16)!=0x40000000u||h[22]||h[23]!=32)return 0;
    for(unsigned i=0;i<3;i++){if(!mpm_read(d,64+i*8208,h,16)||memcmp(h,"CHIP",4)||mpm_u32(h+4)!=0x10200000u||h[10]||h[11]!=(i==2?1:0)||h[12]!=(i==1?0xa0:0x80)||h[13]||h[14]!=0x20||h[15])return 0;}
    if(!mpm_read(d,0x4070,h,256)||memcmp(h,"MGC1",4)||h[4]!=1||h[5]||h[6]||h[7]!=1)return 0;
    expected=mpm_u32(h+56);memset(h+56,0,4);if(mpm_crc(h,256)!=expected)return 0;
    d->count=mpm_u32(h+20);d->module=mpm_u32(h+40);d->content_index=mpm_u32(h+44);
    if(mpm_u32(h+8)!=bytes||mpm_u32(h+12)!=1||mpm_u32(h+16)!=4096||d->count<2||d->count>32||mpm_u32(h+24)!=MPM_PREFIX||mpm_u32(h+28)!=128||!mpm_zero(h+52,4)||!mpm_zero(h+60,4)||!mpm_zero(h+240,16))return 0;
    if(!mpm_text(h+64,24,d->id)||!mpm_text(h+88,64,0)||!mpm_text(h+152,64,d->content)||!mpm_text(h+216,24,0)||!mpm_name(d->content)||d->id[0]<'A'||d->id[0]>'Z')return 0;
    for(const char *p=d->id;*p;p++)if(!((*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9')||*p=='_'||*p=='-'))return 0;
    uint32_t boot=~0u;uint8_t block[256];
    for(unsigned half=0;half<2;half++)for(unsigned at=0;at<8192;at+=sizeof block){if(!mpm_read(d,80+half*8208+at,block,sizeof block))return 0;boot=mpm_crc_step(boot,block,sizeof block);}
    if(~boot!=mpm_u32(h+36)||!mpm_check_crc(d,MPM_PREFIX,d->count*128,mpm_u32(h+32)))return 0;
    cursor=MPM_PREFIX+d->count*128;
    memset(&previous,0,sizeof previous);
    for(uint32_t i=0;i<d->count;i++){
        if(!mpm_entry(d,i,&e)||!e.bytes||e.bytes>MPM_LIMIT||(i&&mpm_compare(previous.name,e.name)>=0))return 0;
        for(uint32_t j=0;j<i;j++){MpmEntry prior;if(!mpm_entry(d,j,&prior))return 0;unsigned k=0;while(prior.name[k]&&mpm_upper(prior.name[k])==mpm_upper(e.name[k]))k++;if(!prior.name[k]&&e.name[k]=='/')return 0;}
        if(!e.flags){if(e.blocks||e.table||e.table_crc||e.offset!=cursor||e.stored!=e.bytes||!mpm_range(cursor,e.bytes,bytes))return 0;cursor+=e.bytes;}
        else{
            if(e.flags!=1||e.blocks!=(e.bytes+4095)/4096||e.table!=cursor||!mpm_range(cursor,e.blocks*16,bytes)||!mpm_check_crc(d,cursor,e.blocks*16,e.table_crc))return 0;
            cursor+=e.blocks*16;if(e.offset!=cursor)return 0;
            for(uint32_t j=0;j<e.blocks;j++){uint8_t b[16];if(!mpm_read(d,e.table+j*16,b,16))return 0;
                uint32_t stored=mpm_u32(b+4),n=stored&0x7fffffffu,decoded=e.bytes-j*4096;if(decoded>4096)decoded=4096;
                if(mpm_u32(b)!=cursor||mpm_u32(b+8)!=decoded||!n||n>decoded||((stored&0x80000000u)?n!=decoded:n>=decoded)||!mpm_range(cursor,n,bytes))return 0;cursor+=n;
            }if(cursor-e.offset!=e.stored)return 0;
        }previous=e;
    }
    if(cursor!=bytes||d->module>=d->count||d->content_index>=d->count||d->module==d->content_index)return 0;
    if(!mpm_entry(d,d->module,&e)||strcmp(e.name,"engine.mvm")||e.flags||!mpm_entry(d,d->content_index,&e)||strcmp(e.name,d->content))return 0;
    return 1;
}
#endif
