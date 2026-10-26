# Firmware installation

Use the [current release](https://github.com/ziggystar12/MHS-GAME-SUPPORT/releases/latest).
Preserve existing games, settings and saves when applying routine updates.

## TeensyROM

Extract Teensy.Support.Package.zip. Flash Firmware/TeensyROM+_0.9_full.hex
with Teensy Loader if setting up the supplied stock core. Copy MPE.TRH, Sys,
APPS and VMS to SD. Install MPE.TRH through the stock text menu, then fully
power-cycle. Keep the three VMS/MPE registration files together.
Installing VMBoot.TRH afterward replaces the single MPE host slot.
Launch a VM .MPE directly or Sys/HAMSTEROS.MPE for the desktop.
[Host details](../docs/MPE-HOST.md).

## A8PicoCart and A8PicoDuo

A8PicoCart-Support.zip contains A8Pico-Factory-1.6.35.uf2. The Duo package
contains A8PicoDuo-A-Factory-1.6.35.uf2 and A8PicoDuo-B-Factory-1.6.35.uf2.
Both include 01-ERASE-ALL-16MiB-Verified.uf2, README.txt and required licenses.
These full factory images preload the matching software and replace all files
and settings on each Pico. Back up an existing cartridge first. Use them for
initial manufacturing or an intentional complete reset; they are not routine
update files.

Follow the included README's BOOTSEL installation directions. Disconnect Atari
power and connect one Pico at a time to USB. First install the supplied eraser
and wait for RPI-RP2 to return after verification. Then install that Pico's
factory image. For Duo, perform both steps separately on A/U1 and B/U2 using
the matching pair. Fully power off/on afterwards. A normal FIRMWARE.UPD cannot
replace the protected Duo startup code. Routine Power Engine updates preserve
user files; these factory downloads intentionally replace them.

Firmware 1.6.35 combines the Type-63 cartridge cache improvement and Duo startup
fixes. Native Monkey Island is withheld and is not included. The merged images
have software and filesystem-readback qualification; physical compatibility
with every Atari or cartridge is not established.

Separate Atari VM ZIPs retain the manufacturer's SD layout: TRITARI.MPE and
GGVM.MPE use the root; Doom uses GAMES/DOOM.MPE on Solo and GAMES/DoomDuo.MPE
on Duo. Read each package's README and preserve existing saves.

## Kung Fu Flash 2

Extract KFF2-Support.zip and select KungFuFlash_v2.29.upd through KFF2's update
menu, then reboot. Copy the matching Sys and APPS folders to SD, preserving
games, settings and saves. Launch Sys/HAMSTEROS.MPE for the optional desktop.
License notices and written source offers accompany the package.

The retained [H24 hardware record](../docs/KFF2-H24.md) documents the older
updater only. Current 2.29 software qualification does not establish physical
acceptance of that newer image.
