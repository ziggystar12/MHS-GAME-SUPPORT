# Current VM packages

Extract each ZIP from the [current release](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest)
to SD and launch its MPE. Preserve VMS libraries and saves when updating.

| Package | Targets | External files |
| --- | --- | --- |
| NESVM.MPE | Teensy, KFF2 | VMS/NESVM/ROMS and SAVES |
| DOOMVM.MPE | Teensy, KFF2 | Licensed shareware included; preserve Saves |

Teensy requires the revised [MPE host](../docs/MPE-HOST.md). NESVM and Doom
carry both checked device engines in one portable MGC1 package.
Console VM inclusion does not promise every ROM is compatible.

a8picocart contains separate Atari firmware, HAMSTEROS.MPE, DOOM.MPE and SYS
companions. HamsterOS for Teensy and KFF2 has its own matching companion ZIP.
Other offered VMs, games and conversion stay in Power Engine;
the unreleased A8 compiler is not distributed here.
