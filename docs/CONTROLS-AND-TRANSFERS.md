# Controls and transfers

Keep each game's supplied C64 client paired with its engine. The client scans
controls and runs the display; the host forwards its input without guessing
what an individual game's buttons mean. The public `VmInput` carries buttons,
display selector, overflow and a protocol marker. Forward that marker unchanged.

## Teensy packet mailbox

The MHS host's packet service uses bank 58 and cartridge I/O2 at $DF00–$DFFF.

| Address | Use |
| --- | --- |
| $DF00–$DFEF | Pending packet bytes |
| $DFF0–$DFF3 | `M3TP` service signature |
| $DFF4 | Client command: 1=start, 3=input ready, 5=border grant |
| $DFF5 | Host status: 2=running, $12=quiet/replay, $E0=failure |
| $DFF6 | Client's acknowledged packet sequence |
| $DFF7 | Host's published packet sequence |
| $DFF8–$DFFA | Input buttons, display and overflow; also failure detail |
| $DFFB | Startup timing / host failure code |
| $DFFC | Host's accepted input sequence |
| $DFFD–$DFFF | Input protocol, new input sequence and checksum |

For input, stage the fields and a new nonzero sequence, then write command 3
last. Its checksum is `$A5 XOR buttons XOR display XOR overflow XOR protocol
XOR sequence`. The host accepts a sequence once and echoes it at $DFFC.
Some fields have different roles during startup and failure; follow the
supplied client rather than writing input during a failure.

A packet starts with `M3`, version 1, type, sequence, flags, payload length
and a zero reserved byte. Up to 228 payload bytes follow, then a little-endian
CRC16 (initial $FFFF, polynomial $1021). Publish the packet sequence last.
The engine must keep a pending packet unchanged until its sequence is
acknowledged. Consume that acknowledgement before pumping the engine again.

## Display DMA

The package-owned transfer request gives the destination spans and ready/done
mailbox. Use the actual [validator](../interfaces/teensy/VMC64Transfer.h) and
[transfer include](../integration/teensy/VMC64TransferHost.h). A border transfer
needs a fresh grant. Read readiness, transfer the complete accepted slice,
write completion and close DMA. A cold upload may use several requests before
the final `LAST` request releases its parked receiver.

Audio and display commits remain ordered by the client/engine protocol.
Do not send a successor packet or reuse its buffers while acknowledgement or
DMA completion is pending. Keep input and the recovery button responsive.

These addresses describe the MHS Teensy service. A KFF2 or other-device host
must use its matching client and platform interface, or supply an explicit
adapter. The shared `.MPE` filename does not define a universal register map.
