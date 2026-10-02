# Firmware installation

Use the [current release](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest).
Preserve existing games, settings and saves.

## TeensyROM

Extract MPE-Host.zip. Flash Firmware/TeensyROM+_0.8.0.13_full.hex with Teensy
Loader. Copy the setup files to SD, install MPE.TRH through the stock text
menu, then fully power-cycle.
Keep all three VMS/MPE registration files. Installing VMBoot.TRH afterward
replaces the single MPE host slot. Launch a VM .MPE directly, or copy the
included Sys/APPS/MPE folders to SD and launch Sys/HAMSTEROS.MPE.
[Host details](../docs/MPE-HOST.md).

## A8PicoCart

A8PicoCart-Support.zip contains shared firmware 1.2.10 and matching applications.
Copy its UF2 to internal cart storage, safely eject, select it in the text menu
or HamsterOS, confirm with Y, wait for verified success and power-cycle. This
requires the protected updater installed since firmware 1.2.0. Initial installs
use the cartridge USB/BOOTSEL procedure.

Put Atari HAMSTEROS.MPE at the root; copy SYS companions together.
Replace DOOM.MPE in your existing Doom folder, retaining its data. These files
belong on the Atari cart. Exact latest physical acceptance remains pending.

## Kung Fu Flash 2

Select KungFuFlash_v2.H24.upd through KFF2's update menu and reboot. Keep your
existing desktop/apps and saves. KFF2-Support.zip includes matching source and
notices, plus the matching optional HamsterOS desktop and apps.
[Browser changes and acceptance](../docs/KFF2-H24.md).
