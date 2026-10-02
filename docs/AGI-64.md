# AGI-64 cartridge support

These games run on the C64. A cartridge supplies the banked game ROM.
MPE's processor host is not needed for these native cartridges.

| Format | CRT type | Banks | ROM windows |
| --- | --- | --- | --- |
| 1MB EasyFlash | 32 | 64 × 16KB | $8000–$9FFF and $A000–$BFFF |
| 2MB MegaCart / Magic Desk 16K | 85 | 128 × 16KB | $8000–$BFFF |

## 1MB EasyFlash

Implement the standard EasyFlash mapping, including Ultimax startup, the
$DE00 bank register, $DE02 mode control and $DF00–$DFFF cartridge RAM.
Each logical bank has an 8KB ROML and 8KB ROMH half. Keep both halves on
the selected bank. The game then runs in 16KB mode.

Build an owned game in this format with the AGI-64 compiler.
[EasyFlash developer information](https://skoe.de/easyflash/files/devdocs/EasyFlash-AppSupport.pdf).

## 2MB MegaCart

Our 2MB cartridges use **Magic Desk 16K, CRT type 85**.
The bank register is $DE00: bits 0–6 select bank 0–127; bit 7 disconnects
the cartridge. Clearing bit 7 restores the selected ROM mapping.
For a full 2MB image, the CRT has 128 CHIP records, each containing 16KB
at load address $8000. The header selects 16KB mode: GAME=0, EXROM=0.
Bank 0 provides the cartridge startup header.

Keep the game code running while banks change. If all banks fit in cartridge
RAM, no bank-loading pause is needed. An SD cache must hold the C64 safely
with /DMA during a missing-bank refill and resume only when the complete
bank is ready. [Our KFF2 implementation](KFF2-2MB.md) shows that path.

Build an owned game in this format with the AGI-64 compiler.
This is a separate format from standard 1MB EasyFlash. A future 2MB EasyFlash
extension would need its own firmware/runtime support and qualification.

## Check your implementation

Start both examples, move between rooms and test keyboard/joystick input.
Check both ROM windows and banks 0, 63, 64 and 127. Repeatedly alternate
low and high banks to exercise cache eviction. Test cartridge disconnect,
re-enable and reset back to the menu.

The included checker validates every CHIP record and the full ROM capacity.
That file check does not replace a physical cartridge test.
