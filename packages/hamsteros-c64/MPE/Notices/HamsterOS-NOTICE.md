# Reuse and notices

HamsterOS_C64 is a Mean Hamster Software project. Its portable document/history and desktop code adapt the behavior contracts audited in the PC and CGA editions; their x86/8086 kernels and executable formats are not copied into this runtime.

The mouse pointer is copied exactly from `HamsterOS_CGA/kernel/cga_kernel.S`, labels `pointer_arrow_mask` and `pointer_arrow_fill_mask`. The spelling service adapts the algorithms in `HamsterOS/apps/hamwrite.c` and retains the exact original dataset and notices under `assets/SYSTEM`.

File and application icons reuse the existing HamsterOS icon library; drive artwork reuses HamsterOS shell glyphs and the C64 GUI's SD/cartridge artwork. Source hashes and the palette conversion are recorded with the imported icon assets.

The target builds use the maintained Mean Hamster Power Engine source and its MPE/Prism+ components. Their existing licenses/notices remain applicable. Teensy distribution includes `LICENSE-NUFLIX.txt` and `NUFLIX-NOTICES.md`; dictionary provenance and ESDB notices accompany `SPELL.DAT`. No commercial game content is part of this desktop package.

The 1.4.5 File Manager, help, troubleshooting and colour app icons are original MHS work. No Commodore Desk implementation code was copied.
