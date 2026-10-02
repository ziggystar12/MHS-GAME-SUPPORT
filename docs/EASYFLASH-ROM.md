# 2MB and 4MB EasyFlash ROM proposal

Experimental read-only ROM support. This is separate from the released firmware.

| ROM size | 16KB banks | Bank mask at `$DE00` |
| --- | --- | --- |
| 1MB | 64 | `$3F` |
| 2MB | 128 | `$7F` |
| 4MB | 256 | `$FF` |

Each bank has 8KB at `$8000` and 8KB at `$A000`. `$DE02`, the 256-byte IO2 RAM,
and reset behavior keep the regular EasyFlash design. Physical cartridge makers
still need to confirm their register design matches this proposal.

## CRT files

Use CRT type 32 and two 8KB CHIP packets per bank. The high half may also use
`$E000`. CHIP bank numbers are the existing 16-bit field.

The highest bank in the file chooses capacity: 0–63 is 1MB, 64–127 is 2MB,
and 128–255 is 4MB. Missing banks or halves read as `$FF`. Keep a CHIP packet in
the final bank, even when it contains only `$FF`, when saving a sparse image.

## Firmware and emulator

The MHS firmware prototype extends Teensy's SD swapping and KFF2's DMA bank
cache. Both need physical testing before release. Larger images are ROM-only;
the regular 1MB EAPI erase, program and save service is not extended.

Stock VICE 3.10 supports 64 EasyFlash banks. The [experimental VICE patch](../integration/vice/README.md)
adds 128 and 256 banks, plus CRT saving and snapshots for the larger ROMs.

## Try a bank test

With Python 3, from this repository:

```sh
python tools/make-easyflash-test.py 2 .build/easyflash-2mb.crt
python tools/make-easyflash-test.py 4 .build/easyflash-4mb.crt
```

These are small diagnostic programs with bank markers. They contain no game.
The [VICE guide](../integration/vice/README.md) shows how to read the markers.

## Games

The AGI compiler prototype offers separate 2MB EasyFlash and 2MB MegaCart choices.
A MegaCart game needs an EasyFlash build; changing its CRT header is not enough.
The AGI packer needs an address/logic-flag change before games can use 4MB.
Cartridge capacity alone does not make those game builds ready.
