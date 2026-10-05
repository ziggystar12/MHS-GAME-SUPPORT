# MPE host

From the stock TeensyROM text menu, install MPE.TRH, then fully power the C64 off and on. Preserve VMS/MPE registration files. Launch VM .MPE files directly; replace each MPE file to update its runtime. This host replaces the extension slot previously occupied by VMBoot.TRH. [Installation details](../../docs/MPE-HOST.md) cover the preserved setup ZIP and registration files.

The updated host, source rebuild and standalone NESVM route passed software checks. The owner confirmed the reported NTSC NES launch failure was fixed on stock TeensyROM+ 0.8.0.14; installed files were not independently read back. Host checks used a rebuilt desktop. The unchanged public desktop and apps with this new host have not been requalified. Full-game and additional hardware acceptance remain unverified.
