# Adding MPE support

MPE runs the game engine on the cartridge processor. The C64 supplies the
display, controls and SID. A working port needs both the cartridge host
and engine builds for that processor.

## Services the host supplies

- Load the selected package and its C64 client; check its target, engine
  interface, memory sizes and required services before starting it.
- Give the engine its code, working memory, guest memory and clock.
- Read package files, keep their paths intact, and write saves beside the
  original package identity. Package files appear under `/@cart`.
- Forward keyboard, joystick and supported mouse input.
- Deliver packets, SID updates and C64 memory transfers in the required order.
- Handle errors, the recovery button, reset and return to the cartridge menu.

## DMA and display transfers

The current C64 transfer path requires **bus-master DMA**, including C64 RAM
reads and writes. Merely holding /DMA to pause the CPU is not enough for MPE
display delivery. Keep VIC-II bus ownership and PAL/NTSC timing intact.

For the newer package-owned rendering path, the game prepares the display
bytes and owns the C64 receiver. The host treats those bytes as transfer
data. It needs no Prism+ conversion algorithm.

A request gives a payload, destination spans, ready/done mailbox and flags.
Validate the whole request before touching the bus. Keep the payload unchanged
while Busy, wait for a fresh receiver grant, copy the spans, publish completion
and release the bus. Finish urgent transfers before starting storage work.
Packets stay frozen until the C64 acknowledges them; replay the pending
packet instead of advancing the engine after a damaged transfer.

The [Teensy headers and transfer reference](../integration/teensy/README.md)
provide the actual interface, limits and transfer sequencing.
[Controls and mailbox details](CONTROLS-AND-TRANSFERS.md) describe the C64 side.

## Current packages

The current VM packages carry their engines and C64 clients. NESVM's Prism+
mode is compiled code. The revised MPE host provides their negotiated services
and preserves the original VMS library/save roots. [Current packages](../packages/README.md).

The public headers describe services and package readers. Prism+ renderer
implementation source and the game packing application stay with MHS.

## Other cartridges

Teensy MPE uses MGC1 containers and MVM1 ARM modules. KFF2 accepts its legacy
KFP1 packages and portable MGC1 packages carrying explicit KFM1 target assets.
The supplied portable VMs contain both targets; a common suffix alone does not
provide a processor port.
[Format readers](../interfaces/README.md) identify and validate each target.

Other cartridges can provide the same host services using their own processor
and hardware. Build engines for that processor and match the package interface.
Native AGI-64 CRT support can be implemented separately.
