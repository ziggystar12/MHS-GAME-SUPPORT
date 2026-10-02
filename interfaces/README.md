# Package and host interfaces

These are the maintained public interface/reader headers, with their original
licenses. They contain no game packer or Prism+ renderer implementation.

| Target | Package reader | Engine/host interface |
| --- | --- | --- |
| TeensyROM+ | [VMGameCart.h](teensy/VMGameCart.h), MGC1 | [VMABI.h](teensy/VMABI.h), ABI 2 / MVM1 |
| KFF2 | [kff_package.h](kff2/kff_package.h), KFP1 and explicit MGC1 targets | [kff_mpe.h](kff2/kff_mpe.h), ABI 8 / KFM1 |

Use the reader's bounds, header checks and member checks before executing
anything. Keep platform formats distinct. MGC1 packages have a CRT-compatible
client prefix plus the package directory and engine/content members. A loader
must inspect the signature instead of assuming every `.crt` is an ordinary ROM.

The Teensy [transfer validator](teensy/VMC64Transfer.h) accepts at most 64
spans and 2048 payload bytes. Destinations must avoid low system memory,
I/O space and the ready/done mailbox. The per-request cost, including span
overhead, is at most 2048 on PAL and 1536 on NTSC. `BORDER` waits for a fresh
receiver grant; `LAST` finishes a cold upload. Follow the header for exact fields.

Implement only services you can provide, report that mask, and reject engines
whose required services or memory profile do not fit. Matching an interface
number alone does not prove compatibility with another host's service layout.
