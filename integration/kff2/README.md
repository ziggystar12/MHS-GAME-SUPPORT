# KFF2 2MB CRT implementation

These files are the MHS KFF2 Type-85 addition with the retained KFF2 license.
The [guide](../../docs/KFF2-2MB.md) explains installation and behavior.

- [magic_desk_16.c](firmware/cartridges/magic_desk_16.c): cache ownership, bank
  switching, safe DMA holds, refill publication and failure recovery.
- [loader.c](firmware/loader.c): CRT scanning, backing-file ownership, initial
  bank loading, foreground refills and the launcher dispatch.
- [md16-test.c](mpe/tests/md16-test.c): host tests of the actual cache handler.

These are integration reference files. Add them to a compatible KFF2 source
tree and wire CRT type 85 into cartridge selection/initialization. The loader
uses the existing FatFs, bus and launcher helpers. This small reference tree
is not a complete firmware build.

To run the cache test from this repository:

```sh
gcc -std=gnu11 -O2 integration/kff2/mpe/tests/md16-test.c -o md16-test
./md16-test
```

The resulting executable is a local test; do not put it on a cartridge.
Upstream firmware: https://github.com/KimJorgensen/KungFuFlash2
