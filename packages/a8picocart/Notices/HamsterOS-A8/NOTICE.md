# Attribution and source boundaries

HamsterOS_A8 is private Mean Hamster Software development. New compact shell and
preview surface code are MHS contributions; no new blanket open-source license
is granted by creating this repository.

The font, desktop icons, pointer silhouette and window-control style are adapted from the
maintained `Mean-Hamster-Power-Engine/desktop/hamsteros` source. Builds retain its
NOTICE.md and exact source hashes in the build's `shared` folder and receipt.
The pointer's CGA provenance remains in the maintained hos_chrome.c; this project
does not build or modify the CGA edition.

The port-2 mouse helper adapts the maintained AGI-ANTIC/native Atari quadrature
decoder behavior. It uses physical port 2, matching the shared joystick-1/mouse-2 default.
No game data or game-specific runtime is copied.

The A8H1 browser uses the established A8PicoCart menu command protocol by
Robin Edwards (Electrotrains). Its RAM-request, cartridge activation and patched
ATR OS-copy sequences follow the upstream Atari Boot ROM. The low-RAM XEX loader
is written in 64tass syntax using the upstream length-prefixed paging protocol
and INITAD/RUNAD behavior, whose upstream implementation credits Jon Halliday
(FJC). Those source obligations must be resolved and included before distribution.
The shared A8 source is the protocol authority; its notice obligations remain.

PrismA8 and A8PicoCart remain shared components of Mean-Hamster-Power-Engine-A8.
Their original notices and component terms must accompany eventual distribution.
This shell's host preview surface does not constitute another PrismA8 renderer.
Toolchains, commercial inputs and hardware firmware snapshots stay out of Git.

IMAGE.LIB reuses the maintained C64 `hos_image.c`, `hos_picture.c` and pinned
HamNet image kernels. The separate image-codecs library remains in shared A8
source. Its build freezes those sources and HamNet LICENSE/ORIGIN notices, with
exact hashes in the image-library receipt. Keep that corresponding source and
its existing proprietary/third-party terms with any private release bundle.
