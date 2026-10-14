# MPE host for stock TeensyROM

[Current Teensy setup](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/Teensy.Support.Package.zip).
The setup ZIP contains the stock text-menu 0.8.0.15 core, matched 1.4.11 MPE.TRH
and HamsterOS desktop, six apps, dictionary and generic .MPE registration.

1. For initial stock setup, flash the supplied 0.8.0.15 full HEX with Teensy Loader.
2. Extract Teensy.Support.Package.zip to SD, preserving libraries and saves.
3. Install MPE.TRH in the stock menu, then fully power the C64 off and on.
4. Launch a VM .MPE or the supplied Sys/HAMSTEROS.MPE desktop.

Keep the supplied VMS/MPE/manifest.vmi, engine.mvm and client.crt together.
Installing VMBoot.TRH replaces the one host slot. A stock entry may display Unk;
registration routes its launch. Each standalone C64 VM contains its engines
and receivers. NESVM, Game Gear and Tritari keep their external libraries under
VMS; both Teensy and KFF2 players are in their portable packages.

Tap the cartridge Menu/reset button after a desktop-launched MPE game to return
to HamsterOS. Hold Menu about two seconds for stock. Direct stock MPE launches
and native PRG/CRT/disk launches return to stock.

The current host and desktop come from the qualified Power Engine 2.1.8 runtime.
The stock 0.8.0.15 core is retained unchanged. Software and package readback
checks are separate from physical cartridge and complete-game acceptance.
Repository host binaries and [verification](../firmware/mpe-host/VERIFICATION.json)
retain their historical identities; use the current setup ZIP for installation.

TeensyROM and its host system are by Travis Smith / Sensorium Embedded; MPE
and HamsterOS by MHS. Required component notices and
[source offers](../source/README.md) accompany downloads.
