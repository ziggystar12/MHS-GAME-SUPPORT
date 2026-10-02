# HamNet image kernels

Mean Hamster Software's `HamNet/core/src/hamnet_image_decode.c` is pinned here
as `image_decode.reference.c`, SHA-256
`a96a51e23ef758518fa5209ba70ae74316acd08a0549815fab0f37c2ea0b8a63`.
The owner's requested C64 port reuses its baseline JPEG and DEFLATE kernels.
`python scripts/adapt-image-decoder.py` reproduces `src/hos_image_kernels.h`.
The C64 adapter quantizes directly to C64 colors, stores packed pixels, bounds
all storage and streams PNG output through a history ring and two scanlines.
The PC/CGA/HamNet source trees remain unchanged. The retained HamNet license
applies to these owner-provided sources.
