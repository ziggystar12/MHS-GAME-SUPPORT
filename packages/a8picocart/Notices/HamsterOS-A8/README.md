# HamsterOS_A8

Private Atari XL/XE desktop for A8PicoCart, using the shared PrismA8-compatible
firmware. Joystick goes in **port 1**, ST-compatible mouse in **port 2**.

The desktop lives in **HAMSTEROS.MPE**. A valid file at the cartridge root starts
automatically; absent or invalid files leave the normal game selector. Compatible
future desktop updates replace the MPE and its matching APP files. PrismA8 presentation remains in game
packages; firmware supplies generic transport, storage and loader support.

HamsterOS 0.4.2 includes:

- Graphics desktop with transparent icons adapted from maintained HamsterOS and
  its cascading About / Programs / Settings / Help / Run / Shutdown menu.
  Cart, D1: and D2: shortcuts select the real storage source. There are
  nine app windows. Move, resize, minimize, maximize/restore and close them;
  the taskbar restores minimized windows.
- Files: cartridge folders, existing game launch routes, documents and images.
  Click or drag the scrollbar. Copy/Paste/Del work on cartridge files; paste
  preserves existing targets and deletion asks for confirmation. Ctrl-C,
  Ctrl-V and Ctrl-Backspace are keyboard equivalents.
- Notepad: insertion editing, cursor movement, wrapping/scrolling, open/save,
  Save As and Save/Discard/Cancel before replacing an unsaved document.
- Calculator: checked integer +, -, *, / with range and divide-by-zero errors.
- Snake: keyboard or joystick 1, pause/restart, score and collision detection.
- Paint: 128 x 112 monochrome canvas, mouse pencil/eraser, clear, image open and
  BMP Save As. P/E select pencil/eraser; N/S/O select new/save/open.
- Image Viewer: BMP, GIF, JPG/JPEG and PNG through the reusable `IMAGE.LIB`.
  It reuses the maintained C64 codecs: first GIF image, baseline JPEG and
  non-interlaced PNG. Images fit to 128 x 112 mono; sources are bounded at
  640 x 480 and compressed input at 24 KB. Uncompressed BMP streams past 24 KB.
- Task Manager: switch to or close real app windows. Control Panel: light/dark
  display themes. About/Help describes the controls.
- Cartridge and Atari D1:/D2: drive selection. SIO supports DOS 2 single-density
  text files; saves create a NEW 8.3 filename and preserve existing disk files.
- Shutdown returns to the normal text launcher without starting HamsterOS again.
- About: The first page shows the running firmware version, queried from the
  cartridge; older hosts show Unknown. Space cycles system, controls and cartridge
  diagnostics, including measured extended RAM, OS revision, PAL/NTSC and installed
  APP/library file counts. Exact Atari model and locked Ultimate 1MB BIOS slots remain unknown.
- Firmware update clicks publish a checking message before validation/staging;
  the existing final Yes prompt still authorizes installation.

Copy `HAMSTEROS.MPE` to the cartridge root. Copy all five `.APP` files and
`IMAGE.LIB` into `SYS`. Apps/libraries prefer `SYS` and still accept legacy root files.
`7800VM.MPE`, games and firmware updates keep their current locations.
Paint, Snake, Calculator, Notepad and Viewer load on demand into 5888 bytes of
Atari scratch RAM. Closing/switching clears their code and data. One scratch app
can be loaded at a time; minimizing retains it. Update the MPE and APPs together.
The image library loads into dormant Pico workspace only during a decode.
It has no dependency on a desktop window and can serve future apps.

Notepad is limited to 2047 bytes; Calculator uses 0..65535 integers. SIO program
launch, other disk formats, disk overwrite and persistent settings are later work.
USB host, networking and SID Player are outside this edition.
Paint saves and file copy/delete currently use cartridge storage. Expanded Atari
RAM is measured but not yet used by these apps. In Ultimate 1MB's full mode,
64 accessible 16 KB banks plus base RAM total 1088 KB. A8PicoCart uses RP2040's
264 KB total shared SRAM; it is not a Teensy or 264 KB of free application memory.

The current ANTIC F desktop is monochrome at 320 x 192. Atari color modes exist;
matching the C64 color scheme needs a separate renderer change. The pointer uses
its top-left arrow tip for clicks, with the PMG/display-list origin accounted for.

```powershell
npm run build:module
npm test
npm run build:firmware
npm run test:firmware
```

The MPE, APPs, library and receipts are in `.build/releases/0.4.2/package`; CPU-rendered previews
are alongside that folder. The normal firmware is built by the maintained
Power Engine A8 project as `A8PicoCart-MPE.uf2`. Its generic application services
support this desktop and future compatible versions. The firmware build has no
dependency on HamsterOS source, UI or application assets. Replace only
the desktop files for compatible desktop updates. Older H1/H2 desktops and games
remain supported by the newer shared firmware. Removing the MPE restores normal startup;
Shutdown returns to the text launcher for the current session.

Source and builds remain private; builds do not flash devices or copy physical
media. Physical Atari/cart/mouse/drive acceptance is still pending.

See [build guide](docs/BUILDING.md), [generic services](docs/A8-APP-SERVICES.md),
[architecture](docs/ARCHITECTURE.md) and [next work](docs/NEXT-SLICE.md).
