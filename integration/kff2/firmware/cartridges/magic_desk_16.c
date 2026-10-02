/*
 * Copyright (c) 2026 Kung Fu Flash 2 contributors
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*
 * Magic Desk 16 uses as many as 128 logical 16k banks. Kung Fu Flash 2 has
 * room for 64 banks in crt_buf, so the loader owns the backing file and uses
 * the foreground refill API below. No filesystem operation is performed from
 * a C64 bus interrupt.
 */
#define MD16_LOGICAL_BANKS  128
#define MD16_CACHE_SLOTS    64
#define MD16_BANK_SIZE      (16*1024)
#define MD16_SLOT_NONE      0xff

typedef enum
{
    MD16_REFILL_IDLE = 0,
    MD16_REFILL_REQUESTED,
    MD16_REFILL_HELD,
    MD16_REFILL_ACTIVE,
    MD16_REFILL_READY,
    MD16_REFILL_FAILED
} MD16_REFILL_STATE;

// Shared between the regular C64 handler and the foreground refill loop.
static volatile u8 magic_desk_16_bank_valid[MD16_LOGICAL_BANKS];
static volatile u8 magic_desk_16_bank_slot[MD16_LOGICAL_BANKS];
static volatile u8 magic_desk_16_slot_bank[MD16_CACHE_SLOTS];
static volatile u32 magic_desk_16_slot_age[MD16_CACHE_SLOTS];
static volatile u32 magic_desk_16_age;
static volatile u8 magic_desk_16_bank_mask;

static volatile u8 magic_desk_16_active_slot;
static volatile u8 magic_desk_16_refill_bank;
static volatile u8 magic_desk_16_refill_slot;
static volatile u8 magic_desk_16_refill_state;
static volatile bool magic_desk_16_started;
static volatile bool magic_desk_16_fatal_error;

void magic_desk_16_handler(void);
void magic_desk_16_dma_handler(void);

/******************************************************************************
* Cache setup API used by the CRT loader before the C64 handler is installed
******************************************************************************/
static void magic_desk_16_cache_reset(void)
{
    magic_desk_16_started = false;
    magic_desk_16_fatal_error = false;
    magic_desk_16_age = 0;
    magic_desk_16_bank_mask = 0;

    magic_desk_16_active_slot = MD16_SLOT_NONE;
    magic_desk_16_refill_bank = MD16_SLOT_NONE;
    magic_desk_16_refill_slot = MD16_SLOT_NONE;
    magic_desk_16_refill_state = MD16_REFILL_IDLE;

    for (u32 bank = 0; bank < MD16_LOGICAL_BANKS; bank++)
    {
        magic_desk_16_bank_valid[bank] = false;
        magic_desk_16_bank_slot[bank] = MD16_SLOT_NONE;
    }

    for (u32 slot = 0; slot < MD16_CACHE_SLOTS; slot++)
    {
        magic_desk_16_slot_bank[slot] = MD16_SLOT_NONE;
        magic_desk_16_slot_age[slot] = 0;
    }
}

// Each entry is zero for an invalid logical bank and non-zero for a bank the
// loader found and validated in the CRT image.
static void magic_desk_16_cache_set_layout(
    const u8 valid[MD16_LOGICAL_BANKS])
{
    magic_desk_16_bank_mask = 0;
    for (u32 bank = 0; bank < MD16_LOGICAL_BANKS; bank++)
    {
        magic_desk_16_bank_valid[bank] = valid[bank] != 0;
        if (valid[bank])
        {
            magic_desk_16_bank_mask = bank;
        }
    }
}

// Publish a foreground result and restart the DMA-timed handler on a fresh
// CC4 boundary. This helper is valid only while the C64 interface is quiesced
// in HELD or ACTIVE, so every terminal foreground path must call it.
static void magic_desk_16_refill_finish(u8 state)
{
    COMPILER_BARRIER();
    magic_desk_16_refill_state = state;
    COMPILER_BARRIER();
    c64_dma_interface_enable_no_config();
}

FORCE_INLINE void magic_desk_16_touch_slot(u8 slot)
{
    magic_desk_16_slot_age[slot] = ++magic_desk_16_age;
}

// Publish a bank which the loader has already copied into crt_banks[slot].
// Duplicate bank or slot assignments are rejected so malformed layouts cannot
// silently alias cache storage.
static bool magic_desk_16_cache_publish_loaded(u8 bank, u8 slot)
{
    if (bank >= MD16_LOGICAL_BANKS || slot >= MD16_CACHE_SLOTS ||
        !magic_desk_16_bank_valid[bank] ||
        magic_desk_16_bank_slot[bank] != MD16_SLOT_NONE ||
        magic_desk_16_slot_bank[slot] != MD16_SLOT_NONE)
    {
        return false;
    }

    magic_desk_16_bank_slot[bank] = slot;
    magic_desk_16_slot_bank[slot] = bank;
    magic_desk_16_touch_slot(slot);
    return true;
}

// Select the initial resident bank after the loader has published its cache.
static bool magic_desk_16_cache_start(u8 initial_bank)
{
    if (initial_bank >= MD16_LOGICAL_BANKS ||
        !magic_desk_16_bank_valid[initial_bank])
    {
        magic_desk_16_fatal_error = true;
        return false;
    }

    u8 slot = magic_desk_16_bank_slot[initial_bank];
    if (slot >= MD16_CACHE_SLOTS)
    {
        magic_desk_16_fatal_error = true;
        return false;
    }

    magic_desk_16_active_slot = slot;
    magic_desk_16_touch_slot(slot);
    crt_ptr = crt_banks[slot];

    magic_desk_16_started = true;
    return true;
}

/******************************************************************************
* Foreground refill API
******************************************************************************/
static bool magic_desk_16_refill_claim(u8 *bank, u8 *slot, u8 **destination)
{
    // The DMA handler first establishes a cycle-aligned hold and quiesces the
    // C64 timer interrupt. Never begin polling the SD controller before that
    // transition has completed.
    if (magic_desk_16_refill_state != MD16_REFILL_HELD)
    {
        return false;
    }

    u8 victim = MD16_SLOT_NONE;

    // Prefer cache storage that has never been populated.
    for (u32 i = 0; i < MD16_CACHE_SLOTS; i++)
    {
        if (magic_desk_16_slot_bank[i] == MD16_SLOT_NONE)
        {
            victim = i;
            break;
        }
    }

    // Otherwise evict the least recently used bank, but never the bank which
    // was active when /DMA stopped the 6510.
    if (victim == MD16_SLOT_NONE)
    {
        u32 oldest_age = 0xffffffff;
        for (u32 i = 0; i < MD16_CACHE_SLOTS; i++)
        {
            if (i != magic_desk_16_active_slot &&
                magic_desk_16_slot_age[i] < oldest_age)
            {
                oldest_age = magic_desk_16_slot_age[i];
                victim = i;
            }
        }
    }

    if (victim == MD16_SLOT_NONE)
    {
        magic_desk_16_refill_finish(MD16_REFILL_FAILED);
        return false;
    }

    magic_desk_16_refill_slot = victim;
    magic_desk_16_refill_state = MD16_REFILL_ACTIVE;
    COMPILER_BARRIER();

    *bank = magic_desk_16_refill_bank;
    *slot = victim;
    *destination = crt_banks[victim];
    return true;
}

// Publish the completed SD read. On failure the old victim mapping is kept;
// the DMA completion handler releases the bus with the cartridge disabled and
// the main loop can observe fatal_error and restart safely.
static bool magic_desk_16_refill_publish(u8 bank, u8 slot, bool success)
{
    if (magic_desk_16_refill_state != MD16_REFILL_ACTIVE ||
        bank != magic_desk_16_refill_bank ||
        slot != magic_desk_16_refill_slot)
    {
        magic_desk_16_refill_finish(MD16_REFILL_FAILED);
        return false;
    }

    if (!success || !magic_desk_16_bank_valid[bank])
    {
        magic_desk_16_refill_finish(MD16_REFILL_FAILED);
        return false;
    }

    u8 old_bank = magic_desk_16_slot_bank[slot];
    if (old_bank < MD16_LOGICAL_BANKS)
    {
        magic_desk_16_bank_slot[old_bank] = MD16_SLOT_NONE;
    }

    magic_desk_16_slot_bank[slot] = bank;
    magic_desk_16_bank_slot[bank] = slot;
    magic_desk_16_touch_slot(slot);

    // READY is published last. The DMA handler cannot expose a partially read
    // cache slot to the C64.
    magic_desk_16_refill_finish(MD16_REFILL_READY);
    return true;
}

static bool magic_desk_16_refill_fatal(void)
{
    return magic_desk_16_fatal_error;
}

/******************************************************************************
* Start a cache miss without doing slow work in the C64 bus ISR
******************************************************************************/
FORCE_INLINE void magic_desk_16_request_refill(u8 bank)
{
    magic_desk_16_refill_bank = bank;
    magic_desk_16_refill_slot = MD16_SLOT_NONE;

    if (magic_desk_16_bank_valid[bank])
    {
        magic_desk_16_refill_state = MD16_REFILL_REQUESTED;
    }
    else
    {
        magic_desk_16_refill_state = MD16_REFILL_FAILED;
    }

    COMPILER_BARRIER();

    // Follow the ordering used by the REU: install and enable the DMA-timed
    // handler before asserting /DMA.
    C64_INSTALL_HANDLER(magic_desk_16_dma_handler);
    C64_DMA_HANDLER_ENABLE();
    C64_CRT_CONTROL(C64_DMA_LOW);
}

/******************************************************************************
* C64 bus read callback
******************************************************************************/
FORCE_INLINE bool magic_desk_16_read_handler(u32 control, u32 addr)
{
    if ((control & (C64_ROML|C64_ROMH)) != (C64_ROML|C64_ROMH))
    {
        C64_DATA_WRITE(crt_ptr[addr & 0x3fff]);
        return true;
    }

    return false;
}

/******************************************************************************
* C64 bus write callback
******************************************************************************/
FORCE_INLINE void magic_desk_16_write_handler(u32 control, u32 addr, u32 data)
{
    // Magic Desk 16 mirrors its register throughout IO1 ($de00-$deff).
    (void)addr;
    if (control & C64_IO1)
    {
        return;
    }

    if (data & 0x80)
    {
        C64_CRT_CONTROL(STATUS_LED_OFF|CRT_PORT_NONE);
        return;
    }

    if (magic_desk_16_fatal_error)
    {
        return;
    }

    // Official images use power-of-two bank counts. Masking reproduces the
    // address-line mirroring used by images smaller than the 2 MiB maximum.
    u8 bank = data & magic_desk_16_bank_mask;
    u8 slot = magic_desk_16_bank_slot[bank];
    if (slot < MD16_CACHE_SLOTS)
    {
        magic_desk_16_active_slot = slot;
        magic_desk_16_touch_slot(slot);
        crt_ptr = crt_banks[slot];

        C64_CRT_CONTROL(STATUS_LED_ON|CRT_PORT_16K);
        return;
    }

    magic_desk_16_request_refill(bank);
}

/******************************************************************************
* DMA hold callback. The foreground loop performs the SD read while this
* handler keeps the 6510 stopped. Completion is applied only while BA is high
* and /DMA is released at phi2 low.
******************************************************************************/
FORCE_INLINE void magic_desk_16_dma_bus_handler(void)
{
    u8 state = magic_desk_16_refill_state;

    if (state == MD16_REFILL_REQUESTED)
    {
        // /DMA has reached a BA-high CPU cycle. Finish that cycle before
        // suppressing the 1 MHz timer interrupt which would otherwise compete
        // with the polling SDMMC transfer. /DMA remains asserted throughout.
        u32 phi2_low = DWT->COMP1;
        COMPILER_BARRIER();
        WAIT_UNTIL(phi2_low);
        C64_INTERFACE_DISABLE();

        COMPILER_BARRIER();
        magic_desk_16_refill_state = MD16_REFILL_HELD;
        return;
    }

    if (state != MD16_REFILL_READY && state != MD16_REFILL_FAILED)
    {
        return;
    }

    bool success = state == MD16_REFILL_READY;
    if (success)
    {
        u8 bank = magic_desk_16_refill_bank;
        u8 slot = magic_desk_16_refill_slot;
        if (bank >= MD16_LOGICAL_BANKS || slot >= MD16_CACHE_SLOTS ||
            magic_desk_16_bank_slot[bank] != slot)
        {
            success = false;
        }
        else
        {
            magic_desk_16_active_slot = slot;
            crt_ptr = crt_banks[slot];
        }
    }

    u32 phi2_low = DWT->COMP1;
    COMPILER_BARRIER();
    WAIT_UNTIL(phi2_low);

    C64_INSTALL_HANDLER(magic_desk_16_handler);
    C64_HANDLER_ENABLE();

    magic_desk_16_refill_bank = MD16_SLOT_NONE;
    magic_desk_16_refill_slot = MD16_SLOT_NONE;
    magic_desk_16_refill_state = MD16_REFILL_IDLE;

    if (success)
    {
        C64_CRT_CONTROL(C64_DMA_HIGH|STATUS_LED_ON|CRT_PORT_16K);
    }
    else
    {
        C64_CRT_CONTROL(C64_DMA_HIGH|STATUS_LED_OFF|CRT_PORT_NONE);
        COMPILER_BARRIER();
        magic_desk_16_fatal_error = true;
    }
}

static void magic_desk_16_init(void)
{
    if (!magic_desk_16_started)
    {
        magic_desk_16_fatal_error = true;
        C64_CRT_CONTROL(C64_DMA_HIGH|STATUS_LED_OFF|CRT_PORT_NONE);
        return;
    }

    C64_CRT_CONTROL(C64_DMA_HIGH|STATUS_LED_ON|CRT_PORT_16K);
}

C64_BUS_HANDLER(magic_desk_16)
C64_DMA_BUS_HANDLER(magic_desk_16)
