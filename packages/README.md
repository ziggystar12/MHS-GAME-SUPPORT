# Current VM packages

Extract each ZIP from the [current release](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest)
using its existing directory layout. Preserve libraries and saves when updating.

| Download | Player and target | External library |
| --- | --- | --- |
| NESVM_C64.zip | NESVM.MPE; Teensy/KFF2 | VMS/NESVM/ROMS and SAVES |
| GBVM_C64.zip | GBVM.MPE; Teensy/KFF2 | VMS/GBVM/ROMS and SAVES |
| DOOMVM_C64.ZIP | DOOMVM.MPE; Teensy/KFF2 | Doom shareware included |
| GGVM_C64.zip | GGVM.MPE; Teensy/KFF2 | VMS/GGVM/ROMS and SAVES |
| Tritari_C64.zip | TRITARI.MPE; Teensy/KFF2 | VMS/TRITARI/ROMS/2600, 5200 and 7800 |
| Tritari_A8.zip | TRITARI.MPE; Atari cartridges | GAMES/7800 |
| GGVM_A8.zip | GGVM.MPE; A8PicoCart | GGVM/ROMS |
| DOOMVM_A8.zip | GAMES/DOOM.MPE; A8PicoCart | Doom shareware included |
| DOOMVM_A8_DUO.zip | GAMES/DoomDuo.MPE; A8PicoDuo | Doom shareware included |

Teensy requires the [MPE host](../docs/MPE-HOST.md). The C64 packages carry
both checked device players. Atari packages use their separate self-contained
format. Supply your own console ROMs; compatibility varies by game.

The A8 support downloads are the complete manufacturer factory-image packages.
Their included README explains initial setup and the full storage replacement.
Teensy and KFF2 desktop/setup files remain in their matching support ZIPs.
Required licenses and source offers accompany downloads; source archives are
not included. Game conversion and other games remain in Power Engine.

The files retained under this repository's packages folders are a historical
integration snapshot. Use the current release assets for current installations.
