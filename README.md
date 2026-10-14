# MHS Game Support

Current cartridge firmware, console VMs, DoomVM and HamsterOS from Mean Hamster Software.
[Download the current support release](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest).
Game conversion and the remaining games live in [Power Engine for 8-Bits](https://meanhamster.com/games/mpe-power-engine).

| Download | Contents |
| --- | --- |
| [Teensy setup](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/Teensy.Support.Package.zip) | Stock 0.8.0.15 firmware, matched 1.4.11 MPE host and desktop, apps, dictionary and MPE registration |
| [KFF2 setup](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/KFF2-Support.zip) | 2.29 updater, matched 1.4.11 desktop, apps and notices |
| [A8PicoCart support](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/A8PicoCart-Support.zip) | Exact A8Pico manufacturer package: 1.5.7 full factory image and installation/license README |
| [A8PicoDuo support](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/A8PicoDuo-Support.zip) | Exact A8PicoDuo manufacturer package: both 1.6.2 full factory images and installation/license README |
| [NESVM C64](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/NESVM_C64.zip) | Portable Teensy/KFF2 player; supply your own ROMs |
| [DoomVM C64](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/DOOMVM_C64.ZIP) | Portable Teensy/KFF2 player with Doom 1.9 shareware |
| [Game Gear VM C64](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/GGVM_C64.zip) | Portable Teensy/KFF2 player; supply your own ROMs |
| [Tritari C64](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/Tritari_C64.zip) | Portable Teensy/KFF2 Atari console player; supply your own ROMs |
| [Tritari A8](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/Tritari_A8.zip) | Atari cartridge console player; supply your own ROMs |
| [Game Gear VM A8](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/GGVM_A8.zip) | A8PicoCart player; supply your own Game Gear ROMs |
| [DoomVM A8](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/DOOMVM_A8.zip) | A8PicoCart Doom package with the shareware episode |
| [DoomVM A8 Duo](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/DOOMVM_A8_DUO.zip) | Dedicated A8PicoDuo Doom package with the shareware episode |

These twelve ZIPs are the uploaded downloads. Existing C64 archive layouts are
retained; the A8 support ZIPs are the completed manufacturer packages unchanged.
No source archives or separate checksum downloads are added. Required component
notices and written source offers accompany the packages.

The A8 factory images replace all files and settings on each Pico. Use them for
initial manufacturing or an intentional complete reset, following the included
README. Back up an existing cartridge first. Routine updates use Power Engine's
matching updater and packages. For ordinary SD/package updates, preserve ROMs,
disks, settings and saves.

The release uses completed component builds and software/file-readback evidence.
Physical cartridge and complete-game acceptance remain separate. Historical H24
browser acceptance applies to that earlier updater, not the current KFF2 2.29 image.
The [component manifest](MANIFEST.json) describes the retained repository snapshot;
the current downloadable packages are the release assets linked above.

- [Installation](firmware/README.md), [MPE host](docs/MPE-HOST.md), [HamsterOS](docs/HAMSTEROS.md)
- [VM packages](packages/README.md), [source and licenses](source/README.md), [checks](tests/README.md)
- [MPE interfaces](interfaces/README.md), [Teensy integration](integration/teensy/README.md)
- [Native AGI carts](docs/AGI-64.md), [Atari carts](docs/ATARI.md), [KFF2 banked carts](docs/KFF2-2MB.md)

TeensyROM is by Travis Smith / Sensorium Embedded; Kung Fu Flash by Kim
Jørgensen and contributors; A8PicoCart by Robin Edwards, with ascrnet updates.
MHS maintains these integrations. Required corresponding source remains available
under the preserved written offers.

[MeanHamster.com](https://meanhamster.com) · [Support development](https://buymeacoffee.com/ziggystar12)
