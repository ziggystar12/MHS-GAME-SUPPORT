// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>

// Trusted-local ARMv7E-M hard-float modules. Not a security sandbox.
// All pointers and callbacks live until reset. Never call from an interrupt.
enum : uint32_t { VM_ABI = 2, VM_CODE_BASE = 0x18000, VM_CODE_LIMIT = 0x30000,
                  VM_DATA_BASE = 0x20014000, VM_DATA_LIMIT = 0x20044000,
                  VM_DATA_BYTES = VM_DATA_LIMIT-VM_DATA_BASE,
                  VM_RAM_BASE = 0x20200000, VM_RAM_BYTES = 512*1024 };
// Optional RAM-only profile: the upper 96 KiB of RAM2 holds initialized,
// non-executable constants; the guest receives only the lower 416 KiB.
// Legacy images keep the entire 512 KiB guest arena and the same ABI/layout.
enum : uint32_t { VM_PROFILE_LEGACY=0, VM_PROFILE_RAM2_RO96=1,
                  VM_PROFILE_RAM1_AUX=2, VM_PROFILE_CODE128=3, VM_PACKAGE_CODE_BASE=0x10000, VM_AUX_CODE_LIMIT=0x28000,
                  VM_RAM2_RO_BYTES=96*1024, VM_RAM2_GUEST_BYTES=VM_RAM_BYTES-VM_RAM2_RO_BYTES,
                  VM_RAM2_RO_BASE=VM_RAM_BASE+VM_RAM2_GUEST_BYTES };
struct VmImageHeader {
    uint32_t magic, abi, header_bytes, code_bytes, data_bytes, bss_bytes;
    uint32_t entry, code_base, ram_base, required_services, payload_crc, header_crc;
    // [0] profile, [1] RAM2 constant payload bytes, [2..3] zero.
    uint32_t reserved[4];
};
static_assert(sizeof(VmImageHeader)==64, "MVM1 image header");
struct VmFileInfo { uint32_t bytes; uint8_t directory; char name[96]; uint8_t attributes; uint16_t date,time; };
// Optional next() handle flag: fail on unrepresentable entries instead of
// skipping them. Old hosts reject the flagged handle without filesystem edits.
enum : uint32_t { VM_NEXT_STRICT = 0x80000000u };
struct VmInput { uint8_t buttons, display, overflow, protocol; };
struct VmPacket { uint8_t type, flags, length, reserved; uint8_t payload[228]; };
// Optional host-owned video transport. VIC_CELL10 is the first, deliberately
// narrow tier: the existing row-major 40x25 representation of eight bitmap
// bytes, screen byte and colour byte per cell. It transfers those three planes
// into an already-established VIC display; background must be zero and HIRES
// must match the current terminal mode. The VM then publishes its ordinary
// frame-end packet, which remains the owner of display-policy and audio commit.
// The submitted bytes remain module-owned and immutable for the duration of
// the synchronous call. Unavailable/Busy/Failed leave ownership with the VM,
// which may retry or use its ordinary packet receiver without changing state.
enum : uint32_t { VM_VIDEO_FORMAT_VIC_CELL10=1, VM_VIDEO_FLAG_HIRES=1 };
enum class VmVideoResult : uint32_t { Unavailable, Transferred, Busy, Failed };
struct VmVideoFrame {
    uint32_t bytes, format, flags, generation;
    uint16_t width, height, stride;
    uint8_t background, reserved;
    const uint8_t *pixels;
};
enum : uint32_t { VM_INDEXED_VIDEO_WORKSPACE_BYTES=24576 };
// Optional setup.reserved geometry flags. Zero preserves prior NES/DOS behavior.
enum : uint16_t { VM_INDEXED_NATIVE_HEIGHT=1, VM_INDEXED_DOUBLE_WIDTH=2 };
// Selectors arrive separately on protocol 90h. Do not interpret ordinary
// input protocol 83h as the older combined controller/video envelope.
enum : uint16_t { VM_INDEXED_SEPARATE_SELECTORS=4 };
// Generic conversion hints, shared by byte-indexed and raster producers.
// Preserve the higher foreground index when shrinking horizontally; use
// palette entry zero as multicolor background; exact CGA RGBI -> VIC colors.
enum : uint16_t { VM_INDEXED_FOREGROUND=8, VM_INDEXED_SOURCE_BACKGROUND=16,
                  VM_INDEXED_RGBI=32 };
// Optional setup hint: F5 uses a fixed half-cell split. Negotiated by
// video_configure; older hosts reject this bit and callers may retry without
// it. Other modes and existing producers retain adaptive conversion.
enum : uint16_t { VM_INDEXED_STABLE_RASTER=64 };
// Negotiated F5 profile for clients with hires sprite-plane support. Older
// hosts reject these bits; producers retry their legacy setup without them.
// SPRITE_TAGS uses bit 6 of a <=64-color index as an actor-priority hint.
// The hint never changes the composited source color, including F1/F7.
enum : uint16_t { VM_INDEXED_SPRITE_F5=128, VM_INDEXED_SPRITE_TAGS=256 };
// Negotiated F3 profile: native 160x200 crop rendered as ordinary multicolor.
// A matching client enables WASD only after receiving crop resume flag 8.
// Combined input 83h then carries W/S/A/D in display bits 4/5/6/7; bits 0..1
// retain the selector and bits 2..3 stay zero. Guest button bits are unchanged.
enum : uint16_t { VM_INDEXED_CROP_F3=512 };
// Optional compact F5: four hires color pairs in a bounded horizontal band.
// Uses VmCenterVideoSetup and 16 KiB scratch. Older hosts reject the extension.
enum : uint16_t { VM_INDEXED_CENTER_F5=1024 };
constexpr uint32_t VM_CENTER_VIDEO_WORKSPACE_BYTES=16384;
// DOS-only experiment: four hires color pairs across the 320x200 picture
// (the final four scanlines share one pair at the VIC badline boundary),
// with the native FLI left-edge artifact exposed (no covering sprites).
// Negotiates separately so existing 16 KiB center-profile modules still work.
enum : uint16_t { VM_INDEXED_FULL_F5=2048 };
constexpr uint32_t VM_FULL_VIDEO_WORKSPACE_BYTES=19456;
enum : uint16_t { VM_INDEXED_NUFLIX_F5=8192 };
constexpr uint32_t VM_NUFLIX_VIDEO_WORKSPACE_BYTES=23328;
// MHS palette-cached multicolor conversion for native 320x200 F1.
// Explicit producer opt-in; other modes retain their established converter.
// Old firmware rejects the bit: retry without it to retain clean F1.
enum : uint16_t { VM_INDEXED_COLOR_F1=4096 };
// Keep the bottom 32 source rows as solid-color UI in Color F1 only.
// Older hosts reject this optional bit; retry without it, then without COLOR.
enum : uint16_t { VM_INDEXED_SOLID_STATUS=16384 };
// Opt-in indexed service: packed RGB palette and row-major 8-bit indices.
// Modes 0 Color, 1 Auto-8, 2 Enhanced-25, 3 Sharp; capability bit = 1<<mode.
// Configuration lends an aligned, lifetime-long RAM1 workspace to firmware.
// Pixels/palette stay immutable across Busy until Transferred (including
// receiver resume ACK). Poll with the SAME generation. Never call from ISR.
// resolved_mode is output only; firmware owns hotkeys and mode selection.
struct VmIndexedVideoSetup {
    uint32_t bytes;void *workspace;uint32_t workspace_bytes;
    uint8_t default_mode,capabilities;uint16_t reserved;
};
struct VmCenterVideoSetup {
    VmIndexedVideoSetup setup; // bytes = sizeof(VmCenterVideoSetup)
    uint8_t first_row,row_count; // center: first 1..14/count 1..9; full: 0/25
    // Prism+ only: 32 promises palette-index-zero margins at x<32/x>=288
    // in a 320x200 source. 1 selects the HamsterOS white sprite pointer;
    // that producer supplies a two-color-per-cell x=264..311 strip and the
    // VmDesktopRasterFrame extension. Zero keeps prior behavior.
    uint16_t reserved;
};
struct VmIndexedFrame {
    uint32_t bytes,generation;
    const uint8_t *pixels,*palette;
    uint32_t pixel_bytes,palette_bytes;
    uint16_t width,height,stride,colors;
    uint8_t resolved_mode;
};
// Optional synchronous raster reader (service 256). Avoids a second native
// framebuffer for banked/packed producers. Firmware calls read_pixel only
// inside video_indexed, never from an ISR or during a later DMA/ACK phase.
// Native backing may change between calls. Keep the descriptor/palette and
// generation stable after source_consumed becomes 1 until Transferred.
// Before consumption a Busy retry may refresh palette/geometry to current
// state. Firmware freezes its converted picture, not the VM's live memory.
struct VmIndexedRasterFrame {
    VmIndexedFrame frame; // bytes = sizeof(VmIndexedRasterFrame); pixels = null
    uint8_t (*read_pixel)(void *context,uint16_t x,uint16_t y);
    void *context;
    uint16_t geometry; // geometry and conversion hints, per frame
    uint8_t reserved,resolved_background;
    uint32_t source_consumed;   // output only, initialize to zero
};
// Optional dirty raster extension, negotiated by the full F5 profile. The
// bitmap uses output coordinates: 40x25 cells, low bit first, 125 bytes.
// Null requests a full conversion. A non-null map may be cleared only after
// source_consumed becomes 1; later guest writes belong to the next picture.
// Palette/geometry changes still invalidate conversion regardless of hints.
struct VmIndexedDirtyRasterFrame {
    VmIndexedRasterFrame raster; // frame.bytes = sizeof(this extension)
    const uint8_t *source_dirty;
};
struct VmDesktopRasterFrame {
    VmIndexedDirtyRasterFrame picture;
    uint16_t pointer_x,pointer_y; // immutable until Transferred; 0..319, 0..199
    uint8_t pointer_color,pointer_flags; // flags bit 0: color supplied; otherwise white
    uint16_t pointer_reserved;
};
enum : uint32_t { VM_OPEN_READ=1,VM_OPEN_WRITE=2,VM_OPEN_CREATE=4,VM_OPEN_EXCLUSIVE=8,VM_OPEN_TRUNCATE=16 };
enum class VmFsOp : uint32_t { Flush,Truncate,Timestamp,Close,Mkdir,Rmdir,Remove,Rename,Space,Media };
// With USB_STORAGE, Media returns the attachment generation in value. USB
// path mutations accept that generation in extra to reject a replaced drive.
struct VmFsRequest { VmFsOp operation; uint32_t handle,value,extra; const char *path,*destination; };
struct VmRamSpan { uint8_t *data; uint32_t bytes; };
enum class VmDesktopAction : uint32_t { Launch=1, Quit=2, Update=3 };
// Desktop-only C64 KERNAL worker. Directory records preserve raw PETSCII:
// name[16], type initial, block count little endian, directory flag.
struct VmDesktopIECRequest {
    uint32_t bytes,operation,offset;
    uint8_t device,name_bytes,count,more;
    uint8_t name[32],entries[160];
};
// Optional desktop-only extension. A static hires snapshot uses ordinary
// VIC fetches while the C64 CPU services IEC. Borrowed synchronously, after
// the Prism raster acknowledges retirement; no additional resident buffer.
struct VmDesktopIECDisplayRequest {
    VmDesktopIECRequest request;
    const uint8_t *bitmap; // 8000 bytes, VIC cell order, copied to $6000
    const uint8_t *screen; // 1000 color pairs, copied to $5c00
};
// Transient settings descriptor. Strings are caller-owned and never retained.
// Operation 0=get, 1=set (number or value input), 2=action, 3=save, 4=describe
// (metadata only, no media access or EEPROM initialization).
// IDs are host-local sequential; -4 means end, id 0xffff/op 3 saves all.
struct VmDesktopSetting {
    char label[24],value[256];
    int32_t number,min,max;
    uint16_t flags;
    uint8_t category,reserved;
};
// Optional foreground desktop tail. Success transfers control and does not
// return. -1 failed, -2 unavailable, -5 frame/packet still owned by receiver.
// Launch accepts an absolute SD PRG/CRT/MPE path; Quit returns to the launcher.
// Optional foreground Ethernet service; buffers remain owned by the desktop.
struct VmDesktopNetStatus { uint8_t phase,link,dhcp,connection,ip[4],mask[4],gateway[4],dns[4]; char error[80]; };
struct VmDesktopNetRequest {
    uint32_t bytes,operation; void *buffer; uint32_t length;
    const char *hostname; uint16_t port,reserved; VmDesktopNetStatus *status;
};
// Immutable scatter transfer; retry the SAME request until Transferred/Failed.
// Firmware retains its pointers while Busy. Packet ACKs and conversion remain
// package-owned. BORDER waits for a fresh receiver grant; LAST releases a cold
// receiver after its final request. A running receiver resumes after each slice.
struct VmC64Span {uint16_t address,offset,count;};
enum : uint32_t {VM_C64_TRANSFER_BORDER=1,VM_C64_TRANSFER_LAST=2};
struct VmC64Transfer {
    uint32_t bytes,flags;const VmC64Span* spans;const uint8_t* payload;
    uint32_t count,payload_bytes;uint16_t ready,done;
    uint8_t ready_value,capability;uint16_t reserved;
};
struct VmHost {
    uint32_t abi, bytes, services;
    uint8_t *workspace; uint32_t workspace_bytes;
    const char *package_root, *content_path;
    uint32_t (*micros_now)();
    // Handles 1..24; zero is failure. read returns -1 on error.
    uint32_t (*open)(const char *path, VmFileInfo *info);
    int32_t (*read)(uint32_t handle, uint32_t offset, void *data, uint32_t count);
    int32_t (*next)(uint32_t directory, VmFileInfo *info); // 1 entry, 0 EOF, -1 error
    void (*close)(uint32_t handle);
    // RAM1 workspace follows module data/BSS. RAM2 is a separate guest arena.
    uint8_t *guest_ram; uint32_t guest_ram_bytes;
    uint32_t (*open_flags)(const char *path,uint32_t flags,VmFileInfo *info);
    int32_t (*write)(uint32_t handle,uint32_t offset,const void *data,uint32_t count);
    // 0 success, -1 failure; Space returns total/free sectors in value/extra,
    // and allocation-unit size (sectors per cluster) in handle.
    int32_t (*file_op)(VmFsRequest *request);
    // Cooperative foreground yield for pending input, ACK, retry or time slice.
    bool (*should_yield)();
    void (*fail)(uint8_t code,uint32_t detail);
    VmVideoResult (*video_present)(const VmVideoFrame *frame);
    // New center-profile hosts accept null to return a completed loan;
    // false means the frame/ACK is still pending and storage remains owned.
    bool (*video_configure)(const VmIndexedVideoSetup *setup);
    VmVideoResult (*video_indexed)(VmIndexedFrame *frame);
    // Profile RAM1_AUX only: non-executable ITCM tail and retired CRT swap
    // RAM. No live FlexRAM repartition, stack or heap borrowing.
    VmRamSpan auxiliary[2];
    int32_t (*desktop)(VmDesktopAction action,const char *path);
    // Real battery-backed RTC seconds since 1970, zero if unavailable.
    // Offset uses the launcher's saved timezone; no network or uptime clock.
    uint32_t (*rtc_now)(int16_t *timezone_minutes);
    // List=1, CD=2, launch=3; stream open/read/write/close=4..8,
    // scratch=9, volume title=10, exclusive rename=11. Foreground only.
    int32_t (*desktop_iec)(VmDesktopIECRequest *request);
    // Optional physical SD volume label; zero success, negative unavailable.
    int32_t (*desktop_volume)(char *out,uint32_t capacity);
    int32_t (*desktop_setting)(uint16_t id,int32_t operation,VmDesktopSetting *setting);
    // Optional desktop-only music lane: chips gate-mask/25-register frames.
    int32_t (*desktop_sid)(const uint8_t *frames,uint32_t chips);
    // Optional confirmed-update handoff. Validation runs after reset in the
    // independent updater; no GUI checksum or completed-frame ACK is required.
    int32_t (*desktop_update)(const char *path,uint32_t bytes);
    // Resident border-scheduled IEC reader. Keeps both Prism pictures and
    // raster interrupts alive. Same requests; operation 0 closes live channels
    // before a native-program handoff. Optional, size checked by the module.
    // On -1, entries may start with "IEC", followed by wire op, first command,
    // completed bytes, status, CIA2 port sample, and uint16 LE elapsed ms.
    // This optional diagnostic describes the first failure before cleanup;
    // count/more are zero. Callers must check the marker before using it.
    int32_t (*desktop_iec_live)(VmDesktopIECRequest *request);
    // STATUS/START/STOP/CONNECT/SEND/READ/CLOSE = 0..6; BUSY=-5.
    // START lends >=96 KiB of guest RAM until STOP; READ 0 means EOF.
    int32_t (*desktop_network)(VmDesktopNetRequest *request);
    // Generic C64 transport for packages carrying their own renderer.
    VmVideoResult (*c64_transfer)(const VmC64Transfer* request);
    uint32_t (*c64_timing)(); // 0 unavailable; 0x80 PAL, 0x81 NTSC
};
// The video callback is a tail extension. Modules which do not require it may
// still run against an ABI-2 host whose VmHost ends immediately before it.
static constexpr uint32_t VM_HOST_BASE_BYTES=offsetof(VmHost,video_present);
struct VmModule {
    uint32_t abi, bytes;
    // pump is permitted while awaiting ACK; it must not alter frozen output.
    void (*input)(const VmInput *input);
    void (*pump)();
    bool (*packet)(VmPacket *out);
    void (*ack)();
};
using VmEntry = const VmModule *(*)(const VmHost *host);
enum : uint32_t { VM_SERVICE_FILES=1, VM_SERVICE_CLOCK=2, VM_SERVICE_PACKETS=4,
                  VM_SERVICE_WRITE=8, VM_SERVICE_GUEST_RAM=16,
                  VM_SERVICE_VIDEO=32, VM_SERVICES=31,
                  VM_SERVICE_INDEXED_VIDEO=64, VM_SERVICE_RAM2_RO=128,
                  VM_SERVICE_INDEXED_RASTER=256,
                  VM_SERVICE_RAM1_AUX=512,
                  VM_SERVICE_PRISM_SPEECH=1024, // Optional runtime-negotiated native-client service.
                  VM_SERVICE_PRISM_SPEECH_DMA=16384, // Checked bulk preload; no C64 address supplied by modules.
                  VM_SERVICE_PRISM_SPEECH_STREAM_DMA=65536, // Experimental checked late-border ring refill; opt-in firmware only.
                  VM_SERVICE_PRISM_SPEECH_DIRECT=131072, // Matched Atlantis: host-owned PCM, concurrent Prism pictures.
                  VM_SERVICE_PRISM_SPEECH_DIRECT_PCM8=262144, // Opt-in SIDKick Pico: unsigned byte PCM, direct protocol 3.
                  VM_SERVICE_SD_ROOT=2048, // Explicit desktop opt-in: ordinary SD paths alongside /@cart.
                  VM_SERVICE_DESKTOP=4096, // Optional native launch/return and explicit quit.
                  VM_SERVICE_TEENSY_CATALOG=8192, // Read-only firmware catalog, independently negotiated.
                  VM_SERVICE_USB_STORAGE=524288, // Desktop /@usb volume; optional, never an SD alias.
                  VM_SERVICE_C64_TRANSFER=1048576, VM_SERVICE_CODE128=2097152,
                  VM_SERVICE_DESKTOP_CURSOR=32768, // Prism+ desktop protocol 3; white hardware pointer.
                  VM_HOST_SERVICES=VM_SERVICES|VM_SERVICE_VIDEO|VM_SERVICE_INDEXED_VIDEO|VM_SERVICE_RAM2_RO|VM_SERVICE_INDEXED_RASTER|VM_SERVICE_RAM1_AUX|VM_SERVICE_SD_ROOT|VM_SERVICE_DESKTOP,
                  VM_KNOWN_SERVICES=VM_HOST_SERVICES|VM_SERVICE_CODE128|VM_SERVICE_C64_TRANSFER|VM_SERVICE_PRISM_SPEECH|VM_SERVICE_PRISM_SPEECH_DMA|VM_SERVICE_PRISM_SPEECH_STREAM_DMA|VM_SERVICE_PRISM_SPEECH_DIRECT|VM_SERVICE_PRISM_SPEECH_DIRECT_PCM8|VM_SERVICE_TEENSY_CATALOG|VM_SERVICE_DESKTOP_CURSOR|VM_SERVICE_USB_STORAGE, VM_IMAGE_MAGIC=0x314d564d };
static inline uint32_t vm_crc32(const void *data, uint32_t size) {
    auto p=static_cast<const uint8_t *>(data); uint32_t c=~0u;
    while(size--) { c^=*p++; for(unsigned b=0;b<8;b++) c=(c>>1)^((0u-(c&1))&0xedb88320u); }
    return ~c;
}
static inline uint32_t vm_image_ro_bytes(const VmImageHeader &h){return h.reserved[1];}
static inline uint32_t vm_image_guest_bytes(const VmImageHeader &h){return h.reserved[0]==VM_PROFILE_RAM2_RO96?uint32_t(VM_RAM2_GUEST_BYTES):uint32_t(VM_RAM_BYTES);}
static inline uint32_t vm_image_payload_bytes(const VmImageHeader &h){return h.code_bytes+h.data_bytes+vm_image_ro_bytes(h);}
static inline bool vm_valid_header(const VmImageHeader &h, uint32_t file_bytes) {
    const uint32_t codeBase=h.reserved[0]==VM_PROFILE_CODE128?uint32_t(VM_PACKAGE_CODE_BASE):uint32_t(VM_CODE_BASE);
    if(h.reserved[2]||h.reserved[3])return false;
    if(h.reserved[0]==VM_PROFILE_RAM1_AUX){
        if(h.reserved[1]||!(h.required_services&VM_SERVICE_RAM1_AUX)||
           (h.required_services&VM_SERVICE_RAM2_RO)||h.code_bytes>VM_AUX_CODE_LIMIT-VM_CODE_BASE)return false;
    }else if(h.required_services&VM_SERVICE_RAM1_AUX)return false;
    else if(h.reserved[0]==VM_PROFILE_LEGACY){
        if(h.reserved[1]||(h.required_services&VM_SERVICE_RAM2_RO))return false;
    }else if(h.reserved[0]==VM_PROFILE_RAM2_RO96){
        if(!(h.required_services&VM_SERVICE_RAM2_RO)||!h.reserved[1]||h.reserved[1]>VM_RAM2_RO_BYTES)return false;
    }else if(h.reserved[0]==VM_PROFILE_CODE128){
        if(h.reserved[1]||!(h.required_services&VM_SERVICE_CODE128)||(h.required_services&VM_SERVICE_RAM2_RO))return false;
    }else return false;
    if((h.required_services&VM_SERVICE_CODE128)&&h.reserved[0]!=VM_PROFILE_CODE128)return false;
    if(h.magic!=VM_IMAGE_MAGIC || h.abi!=VM_ABI || h.header_bytes!=sizeof h ||
       h.code_base!=codeBase || h.ram_base!=VM_DATA_BASE || !h.code_bytes ||
       h.code_bytes>VM_CODE_LIMIT-codeBase || h.data_bytes>VM_DATA_BYTES ||
       h.bss_bytes>VM_DATA_BYTES-h.data_bytes || (h.required_services&~VM_KNOWN_SERVICES) ||
       file_bytes!=sizeof h+vm_image_payload_bytes(h) || !(h.entry&1) ||
       (h.entry&~1u)<codeBase || (h.entry&~1u)>=codeBase+h.code_bytes) return false;
    VmImageHeader check=h; check.header_crc=0;
    return vm_crc32(&check,sizeof check)==h.header_crc;
}
