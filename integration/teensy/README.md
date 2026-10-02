# TeensyROM integration

Keep the vendor's normal menu and cartridge behavior. Route MPE selections
to a dedicated host that supplies the services in [MPE.md](../../docs/MPE.md).

The [MPE.TRH release](../../docs/MPE-HOST.md) uses the upstream extension-host
system. It adapts the launch record, descriptor, memory layout and return path
to the MHS package/module interface. Its installer and registration files live
in [firmware/mpe-host](../../firmware/mpe-host/README.md).

[Upstream extension-host guide](https://github.com/SensoriumEmbedded/TeensyROM/blob/main/docs/Architecture/Extension-Hosts.md).

## Integration steps

1. Preserve the selected file's full path and distinguish MGC1 containers
   from native CRTs. Keep 1MB/2MB native CRTs on the vendor's cartridge path.
2. Validate the package and module using [the interface headers](../../interfaces/README.md).
3. Reserve the exact memory profile from the engine header. Provide the
   required services or refuse the launch cleanly.
4. Load the C64 client, establish PAL/NTSC timing, then start the engine.
5. Service input and acknowledgements; keep output frozen until acknowledged.
6. Run C64 transfer work before slower storage work, and return safely to
   the normal menu on reset, recovery or failure.

## DMA reference

[VMC64TransferHost.h](VMC64TransferHost.h) is the actual generic transfer
implementation used by the MHS Teensy host. It contains bus transfer work,
not a renderer. It is an integration include, not a standalone firmware.

Its includer supplies the host's timing/grant state, memory-range validation,
DMA read/write/close functions, clock and emergency bus release. The current
implementation allows a five-second request deadline and short, bounded
border grants. Retain the same request and payload until completion.

The portable validator lives in [VMC64Transfer.h](../../interfaces/teensy/VMC64Transfer.h).
The included host test replaces the physical bus with memory and exercises
the real implementation. A passing test does not establish cartridge timing.

Existing NESVM/DoomVM also require the legacy indexed-video services. MHS
supplies that compiled runtime; it is separate from this transfer-only reference.
