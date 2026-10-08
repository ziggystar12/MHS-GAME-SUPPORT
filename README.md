# MHS Game Support

Current cartridge firmware, NESVM, DoomVM and HamsterOS from Mean Hamster Software.
[Download the current support release](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest).
Game conversion and the remaining games live in [Power Engine for C64](https://meanhamster.com/games/mpe-power-engine).

| Download | Contents |
| --- | --- |
| [Teensy setup](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/Teensy.Support.Package.zip) | Stock firmware, MPE.TRH, HamsterOS, apps and dictionary |
| [NESVM](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/NESVM.zip) | Portable Teensy/KFF2 MPE player; supply your own games |
| [DoomVM](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/DOOMVM.zip) | Portable player with licensed Doom 1.9 shareware |
| [A8PicoCart support](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/A8PicoCart-Support.zip) | Shared firmware 1.2.10, HamsterOS 0.4.2, Doom launcher and SYS files |
| [KFF2 setup](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest/download/KFF2-Support.zip) | Accepted 2.H24 updater, HamsterOS, matching apps and notices |

The five setup/player ZIPs are the only uploaded downloads.
Each ZIP contains the required companions and license notices.
Preserve games, ROMs, disks,
settings and saves. Atari packages use their own formats. The A8 compiler is a
separate unreleased product and is not included.

The updated MPE host and NESVM passed software checks. The owner confirmed the
reported NTSC NES launch failure was fixed on TeensyROM+ 0.8.0.14. Installed
files were not independently read back; full-game and additional hardware
acceptance remain unverified. The Teensy ZIP supplies stock 0.8.0.15, a matched host and the three VMS/MPE
registration files; its desktop and apps are preserved. Host checks used a rebuilt desktop; the preserved desktop with
the new host has not been requalified. Latest A8 components passed software
checks; physical acceptance of their updated combination remains pending.
KFF2 H24 retains its existing browser acceptance tied to the exact updater hash.
[Component manifest](MANIFEST.json) identifies current files.

- [Installation](firmware/README.md), [MPE host](docs/MPE-HOST.md), [HamsterOS](docs/HAMSTEROS.md)
- [VM packages](packages/README.md), [source and licenses](source/README.md), [checks](tests/README.md)
- [MPE interfaces](interfaces/README.md), [Teensy integration](integration/teensy/README.md)
- [Native AGI carts](docs/AGI-64.md), [Atari carts](docs/ATARI.md), [KFF2 banked carts](docs/KFF2-2MB.md)

TeensyROM is by Travis Smith / Sensorium Embedded; Kung Fu Flash by Kim
Jørgensen and contributors; A8PicoCart by Robin Edwards, with ascrnet updates.
MHS maintains these integrations. Older downloads and examples are removed from
the current page; required corresponding source remains available under the
preserved offers.

[MeanHamster.com](https://meanhamster.com) · [Support development](https://buymeacoffee.com/ziggystar12)
