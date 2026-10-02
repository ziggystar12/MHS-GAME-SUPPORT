# AGI-ANTIC / Atari 8-bit

The Atari example is a **1MB Atarimax CAR, type 42**. It runs the game on
the Atari; no MPE processor host is needed.

Keep the 16-byte `CART` header and its payload checksum. The ROM has 128
banks of 8KB at $A000–$BFFF. Startup selects bank 127. Reads or writes at
$D500–$D57F select the bank from the address's low seven bits; the data
byte is ignored. $D580–$D5FF disables the cartridge's RD5 mapping.

Use an owned native AGI cartridge from the separate Atari compiler and the
[Winbond A8PicoCart firmware](../firmware/README.md). Preserve its full size,
type and checksum. Test launch, room changes,
controls and repeated bank switching on the Atari.

[A8PicoCart source and build instructions](../source/README.md).
