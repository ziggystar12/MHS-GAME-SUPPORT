# Experimental VICE support

This patch adds read-only 2MB and 4MB EasyFlash ROMs to VICE 3.10. It is an MHS
prototype, not an upstream VICE feature.

Apply it to the [VICE 3.10 source](https://github.com/VICE-Team/svn-mirror/releases/tag/3.10.0):

```sh
patch -p1 < easyflash-rom-2m-4m.patch
```

Then build VICE using its supplied instructions. The patch changes only
`src/c64/cart/easyflash.c`. It does not include ROMs or a VICE executable.

## Bank checks

Generate a diagnostic CRT using the [format guide](../../docs/EASYFLASH-ROM.md).
Load it in the patched C64 emulator and enter its monitor:

```text
bank cpu
> $01 $37
> $de02 $87
> $de00 $7f
m $8000 $8003
m $a000 $a003
```

Bank 127 reads `7f 00 80 4d` at `$8000` and `7f 01 80 4d` at `$A000`.
For 4MB, writing `$FF` to `$DE00` selects bank 255. Its markers are
`ff 00 00 4d` and `ff 01 00 4d`. Regular 1MB images still mask that write to bank 63.

## Tested

The Windows headless build passed all 256 bank-register writes for each size,
both ROM halves, IO2 RAM, snapshot restore, and CRT save/reload. It also rejected
bank 256, duplicate halves, truncated data, invalid addresses, packet overruns
and the classic EAPI in larger images. This checks emulator behavior, not a
physical cartridge.

The wider images do not emulate flash erase/program commands or extend EAPI.
Existing 1MB flash behavior stays in place. Sparse CRT saves retain the final
bank so the file keeps its capacity. VICE also continues to accept its existing
16KB CHIP packet layout; the firmware proposal uses paired 8KB packets.

VICE is GPL-2.0-or-later, by the VICE team and its contributors. The patched
EasyFlash source is by ALeX Kazik and Marco van den Heuvel. MHS modifications are
under the same license; see [COPYING](COPYING).
