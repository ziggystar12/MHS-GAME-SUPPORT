/* Host execution of the exact Type-85 cache and bus callbacks from fac4317. */
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;
static u8 memory[64][16384],*crt_banks[64],*crt_ptr,data_out;
static u32 control_out,interface_disabled,resumes,waits,handler_enables,dma_enables;
static void (*handler)(void);
static struct {u32 COMP1;} dwt={249};
#define DWT (&dwt)
#define FORCE_INLINE static inline
#define COMPILER_BARRIER() __asm__ volatile("" ::: "memory")
#define WAIT_UNTIL(v) (waits++)
#define C64_IO1 1u
#define C64_ROML 2u
#define C64_ROMH 4u
#define C64_DMA_LOW 8u
#define C64_DMA_HIGH 16u
#define STATUS_LED_OFF 32u
#define STATUS_LED_ON 64u
#define CRT_PORT_NONE 128u
#define CRT_PORT_16K 256u
#define C64_DATA_WRITE(v) (data_out=(v))
#define C64_CRT_CONTROL(v) (control_out=(v))
#define C64_INSTALL_HANDLER(v) (handler=(v))
#define C64_DMA_HANDLER_ENABLE() (dma_enables++)
#define C64_HANDLER_ENABLE() (handler_enables++)
#define C64_INTERFACE_DISABLE() (interface_disabled=1)
#define C64_BUS_HANDLER(name) void name##_handler(void){}
#define C64_DMA_BUS_HANDLER(name) void name##_dma_handler(void){}
static void c64_dma_interface_enable_no_config(void){assert(interface_disabled);interface_disabled=0;resumes++;}
#include "../../firmware/cartridges/magic_desk_16.c"
static void fill(u8 *p,u8 bank){for(unsigned i=0;i<16384;i++)p[i]=(u8)(bank+i*13);}
static void setup(unsigned banks){magic_desk_16_cache_reset();u8 valid[128]={0};for(unsigned i=0;i<banks;i++)valid[i]=1;magic_desk_16_cache_set_layout(valid);
 for(unsigned i=0;i<64;i++){crt_banks[i]=memory[i];if(i<banks){fill(memory[i],i);assert(magic_desk_16_cache_publish_loaded(i,i));}}
 assert(magic_desk_16_cache_start(0));magic_desk_16_init();assert(control_out==(C64_DMA_HIGH|STATUS_LED_ON|CRT_PORT_16K));
}
static unsigned misses;
static void select_bank(u8 bank){u8 old=magic_desk_16_active_slot;magic_desk_16_write_handler(0,0xde53,bank);
 if(magic_desk_16_refill_state==MD16_REFILL_REQUESTED){misses++;assert(handler==magic_desk_16_dma_handler&&control_out==C64_DMA_LOW);
  u8 b,s,*dest;assert(!magic_desk_16_refill_claim(&b,&s,&dest));
  magic_desk_16_dma_bus_handler();assert(interface_disabled&&magic_desk_16_refill_state==MD16_REFILL_HELD);
  assert(magic_desk_16_refill_claim(&b,&s,&dest)&&b==bank&&s!=old);assert(!magic_desk_16_refill_claim(&b,&s,&dest));
  fill(dest,b);assert(magic_desk_16_refill_publish(b,s,true));assert(!interface_disabled&&magic_desk_16_refill_state==MD16_REFILL_READY);
  assert(magic_desk_16_active_slot==old);magic_desk_16_dma_bus_handler();assert(handler==magic_desk_16_handler);
 }
 assert(magic_desk_16_refill_state==MD16_REFILL_IDLE&&!magic_desk_16_refill_fatal());
 assert(crt_ptr==crt_banks[magic_desk_16_bank_slot[bank]]);
 assert(magic_desk_16_read_handler(C64_ROMH,0x8000)&&data_out==bank);
 assert(magic_desk_16_read_handler(C64_ROML,0xbfff)&&data_out==(u8)(bank+16383*13));
}
int main(void){setup(128);
 assert(!magic_desk_16_cache_publish_loaded(0,63));assert(!magic_desk_16_cache_publish_loaded(128,0));assert(!magic_desk_16_cache_publish_loaded(100,64));
 for(unsigned pass=0;pass<4;pass++)for(unsigned i=0;i<128;i++)select_bank((u8)(pass&1?127-i:i));
 // Every mirrored IO1 register, disable/re-enable and unrelated IO access.
 for(unsigned addr=0;addr<256;addr++){magic_desk_16_write_handler(0,0xde00+addr,0x80);assert(control_out==(STATUS_LED_OFF|CRT_PORT_NONE));select_bank(0);}
 u8 *before=crt_ptr;magic_desk_16_write_handler(C64_IO1,0xdf00,12);assert(crt_ptr==before);assert(!magic_desk_16_read_handler(C64_ROML|C64_ROMH,0x4000));
 for(unsigned banks=2;banks<=64;banks*=2){setup(banks);magic_desk_16_write_handler(0,0xde00,127);assert(crt_ptr==crt_banks[banks-1]);}
 // Failed SD reads must finish through the DMA callback before reporting fatal.
 setup(128);magic_desk_16_write_handler(0,0xde00,127);magic_desk_16_dma_bus_handler();u8 bank,slot,*dest;assert(magic_desk_16_refill_claim(&bank,&slot,&dest));assert(!magic_desk_16_refill_publish(bank,slot,false));assert(!magic_desk_16_refill_fatal());magic_desk_16_dma_bus_handler();assert(magic_desk_16_refill_fatal()&&control_out==(C64_DMA_HIGH|STATUS_LED_OFF|CRT_PORT_NONE));
 assert(misses>250&&waits&&resumes&&handler_enables&&dma_enables);
 printf("PASS actual MD16 source: 512 bank selections, %u misses, active-slot protection, 256 IO mirrors, smaller-image masks, DMA quiesce/resume, failed SD recovery\n",misses);
}
