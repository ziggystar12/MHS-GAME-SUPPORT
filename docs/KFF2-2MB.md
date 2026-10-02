# Our KFF2 2MB CRT support

The MHS KFF2 firmware adds **2MB Magic Desk 16K cartridges (CRT type 85)**.
Use the [current H24 firmware](../firmware/kff2/KungFuFlash_v2.H24.upd). This is an
addition in our firmware; it is not a claim about every stock KFF2 version.

## Use it

1. Install the updater using the KFF2 menu.
2. Copy an owned 2MB Magic Desk 16K CRT built with AGI-64 to SD.
3. Select the CRT in the normal KFF2 cartridge menu.

Keep the SD card inserted while playing. If HamsterOS is installed, SPECIAL
returns to the original launcher, where the CRT can be selected.

## How it fits

A 2MB image has 128 logical banks of 16KB. KFF2's cartridge buffer holds
64 banks at once. The loader keeps the CRT open on SD and remembers where
each bank is stored.

A cached bank switches immediately. For a missing bank, the bus handler
asserts /DMA and safely parks the C64. The foreground code reads a complete
16KB bank into a cache slot. Only then does the handler select that slot
and release /DMA. SD reads never run inside the C64 bus interrupt.

The mapper mirrors its bank register throughout $DE00–$DEFF. Bits 0–6 select
the bank; bit 7 disconnects both ROM windows. The loader rejects missing,
duplicate, out-of-range and malformed banks. Smaller supported images use
a complete power-of-two bank set. A failed refill releases DMA safely and
returns to the menu.

The CRT cache, MPE runtime and REU can share memory at different times.
Stop the previous service, close its files and reset its memory ownership
before launching another mode. Do not reserve the same buffer for a full
REU and the 2MB CRT cache at the same time.

## Code and checks

[The cache/bus handler and loader](../integration/kff2/README.md) are included
with their original notices. The host test runs the actual cache handler
through all 128 banks, repeated evictions, register mirroring and failed
reads. It verifies the callback ordering and safe DMA release.

The firmware and examples retain their existing software qualification.
This publication does not add a new physical KFF2 test or a complete SQ0
playthrough. Check upper-bank access and room transitions on your hardware.
