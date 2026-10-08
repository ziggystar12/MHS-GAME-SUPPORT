# MPE host for stock TeensyROM

[Current Teensy setup](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/Teensy.Support.Package.zip).
Use MPE.TRH with stock firmware on TeensyROM+ v0.4 / Teensy 4.1. The setup ZIP
contains the stock text-menu 0.8.0.15 core, matched MPE.TRH and generic .MPE
registration. Its desktop, six apps and dictionary are unchanged.

1. Flash the supplied stock 0.8.0.15 full HEX with Teensy Loader.
2. Extract Teensy.Support.Package.zip to SD, preserving libraries and saves.
3. Install MPE.TRH in the stock menu, then fully power the C64 off and on.
4. Launch a VM .MPE, or launch the supplied Sys/HAMSTEROS.MPE desktop.

The three [VMS/MPE files](../firmware/mpe-host/VMS/MPE) register .MPE and are now
included in the setup ZIP. Copy the supplied trio together. VMBoot.TRH would
replace the one host slot. A stock entry may display Unk; registration routes its launch.
Each standalone VM carries its engine and C64 receiver. The checked VMBOOT.BIN
configuration opens its original /VMS/<VM> library/save root. Renaming a CRT
does not create an MPE package. The public NESVM package also contains a KFF2 target.

Tap the cartridge Menu/reset button after a desktop-launched MPE game to return
to HamsterOS. Hold Menu about two seconds for stock. Direct stock MPE launches
and native PRG/CRT/disk launches return to stock.

The host supplies rendering, input, transfer, storage, Ethernet, native APP and
PCM8 services. It fits the 384 KiB slot and was built against stock source commit
28f6aa87d763d0d34dc7320fcb44c336850f0323. The stock core uses the same upstream
revision and excludes the separately installed extension-host slot. Host, service,
164 native boot checks and an exact matching-source rebuild passed. The host and service
tests used a rebuilt desktop; the preserved public desktop and apps with this
new host have not been requalified.

The owner confirmed the reported NTSC NES launch failure was fixed with stock
0.8.0.14. Installed files were not independently read back; full-game and
additional hardware acceptance remain unverified. [Current verification](../firmware/mpe-host/VERIFICATION.json)
binds the exact files and keeps this launch report separate from software proof.

TeensyROM and its host system are by Travis Smith / Sensorium Embedded; MPE
and HamsterOS by MHS. [Notices](../firmware/mpe-host/Notices) and
[source offers](../source/README.md) accompany downloads.
