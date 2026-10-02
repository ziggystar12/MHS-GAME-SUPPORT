# Atari files on A8PicoCart

The combined MPE firmware keeps the original cartridge and disk routes and
adds executable aliases and standard boot cassette files. Use either the
text menu or HamsterOS. Names are case-insensitive; keep the original files.

| File | Route |
| --- | --- |
| `.COM`, `.XEX` | Atari DOS binary executable |
| `.BIN` | Valid Atari executable, otherwise a headerless cartridge |
| `.ROM` | Headerless cartridge; existing 7800-header detection is preserved |
| `.CAR` | Cartridge with its explicit mapper header |
| `.CAS` | Standard 600-baud, self-contained boot cassette |
| `.ATR` | Existing limited disk-image route |

Executables must contain complete binary-load segments. Their existing
128 KiB workspace includes a four-byte length prefix, leaving 131,068 bytes
for the file. INITAD and RUNAD records retain their ordinary meaning.
XEX and COM retain the loader's support for ordinary first segments without
an initial signature. BIN detection requires the executable's FFFF signature.

Headerless cartridges can be 2, 4, 8, 16, 32, 64 or 128 KiB. The 2 and 4 KiB
images use the upper portion of the normal 8 KiB cartridge window. Larger
images retain the existing size-based XEGS mapping. Use a `.CAR` file when a
game requires a different bank-switching mapper. Type 42 support remains.
For `.BIN`, a complete valid executable takes precedence over raw size.

The cassette reader validates FUJI chunks, standard data records, checksums
and the boot header before preparing a temporary executable in cart RAM.
The bootstrap moves the complete boot image to its original address and
preserves the cassette continuation and initialization sequence. It does
not write a converted file or alter the original tape image.

This cassette route supports the declared boot image, including the
256-record form. BASIC `CSAVE`, data-only tapes, extra loading records and
turbo/pulse recordings are rejected with an error. Programs that request
further cassette input after boot remain unsupported.
Disk-image support retains the upstream limitations; this update does not
provide a general SIO drive emulator.

`A8FWSTG.BIN` is reserved for firmware recovery. It remains hidden and cannot
be launched as a cartridge. Firmware updates still select a complete `.UF2`
from internal storage. Existing MPE packages keep their stable interfaces.

The base [A8PicoCart project](https://github.com/ascrnet/A8PicoCart) provides
ROM/CAR/XEX and limited ATR support. The new routes are additive to that
maintained source. Software and emulator tests are distinct from physical
Atari timing and complete-game acceptance.
