# MPE host for stock TeensyROM

[Current host](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/MPE-Host.zip).
Use the paired stock full firmware and MPE.TRH on TeensyROM+ v0.4 / Teensy 4.1.

1. Flash the supplied stock full HEX with Teensy Loader.
2. Extract MPE-Host.zip to SD, preserving libraries and saves.
3. Install MPE.TRH in the stock menu, then fully power the C64 off and on.
4. Launch a VM .MPE, or install the desktop ZIP and launch Sys/HAMSTEROS.MPE.

The three VMS/MPE files register .MPE. VMBoot.TRH would replace the one host
slot. A stock entry may display Unk; registration routes its launch.
Each standalone VM carries its engine and C64 receiver. The checked VMBOOT.BIN
configuration opens its original /VMS/<VM> library/save root. Renaming a CRT
does not create an MPE package. The public NESVM package also contains a KFF2 target.

Tap the cartridge Menu/reset button after a desktop-launched MPE game to return
to HamsterOS. Hold Menu about two seconds for stock. Direct stock MPE launches
and native PRG/CRT/disk launches return to stock.

The host supplies rendering, input, transfer, storage, Ethernet, native APP and
PCM8 services. It fits the 384 KiB slot and uses stock source commit
018641a1fce70eba94c49c940ae677758cde0288. Package readers, external libraries,
cold saves, desktop/apps and storage/network lifecycle passed software checks.
The earlier host was owner-tested; acceptance does not transfer to this revised
binary. [Current verification](../firmware/mpe-host/VERIFICATION.json) marks
physical acceptance pending and binds the exact files.

TeensyROM and its host system are by Travis Smith / Sensorium Embedded; MPE
and HamsterOS by MHS. [Notices](../firmware/mpe-host/Notices) and
[source offers](../source/README.md) accompany downloads.
