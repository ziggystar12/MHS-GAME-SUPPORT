# External application and storage services, revision 1

These are generic cart services for external applications. No desktop UI, app
behavior, font, artwork, themes, Notepad, Calculator or Snake is built into firmware.
The existing H1 desktop and stock/alternative menu retain their old behavior.
The implementation lives in the maintained shared firmware's `a8picocart/source`:
`app_package`, `app_storage`, `hos_package` and `app_services.cmake`. The shared
root build enables it without reading this desktop's source. The earlier H1
package is preserved for admission regression checks.

## A8H2 package

The checked 64-byte header retains the H1 fields, with magic A8H2 and ABI 2.
Client offset is 64 and client size is 8192. Total size is 64 plus 8192..32768
payload bytes. The payload CRC covers all payload bytes, not just the boot bank.
Mouse capability is 1 or 2; display profile is 2. Header offset 40 is segment
count (1..4), offset 44 is service revision 1, and 48..63 are zero.

The first 8192 payload bytes are a standard checked boot bank. The original
28-page core relocation guard remains. At payload offset 8192 is an AHS2 segment
directory: magic (4), count (1), reserved zero (3), then count 12-byte records.
A record is payload offset (u32), Atari RAM address (u16), bytes (u16), CRC32 (u32).
Records are ordered, contiguous and consume the rest of the payload exactly.
Segments must fit without overlap in $1000..$1FFF, $8800..$8FFF or $9200..$9BFF.
These are separately guarded application regions; the two bitmap frames, PMG,
display lists and core do not move. No app code or data exceeds the 64 KB target.

Firmware validates the whole package in existing cart workspace before changing
the menu. Before directory enumeration reuses that workspace, the RAM bootstrap
reads the directory and resident segments through command $20. H1 remains accepted
by the existing validator. Future compatible desktop releases change the MPE file.

## Mailbox

Requests execute from Atari RAM. Parameters are $D500..$D5DE; write command to
$D5DF and poll $D500 for $11 as before. Status is $D501, 0 means success.
File/space integers are unsigned little endian. Strings are bounded and NUL
terminated. Unknown commands must not corrupt menu, directory or open writes.

| Command | Request | Reply after status |
| --- | --- | --- |
| $20 Payload block | payload offset u32 at 0, length 1..128 at 4 | returned count at 2, bytes at 3 |
| $30 Enumerate all | current directory supplied by shared caller | count at 2; existing 256-byte directory records |
| $31 Metadata | entry index at 0 | size u32 at 2, attributes at 6, canonical path at 7 (max 215 bytes incl NUL) |
| $32 Read file | index at 0, offset u32 at 1, length 1..128 at 5 | returned count at 2, bytes at 3 |
| $33 Begin write | total bytes u16 at 0, replace flag at 2, path at 3 (max 220 bytes incl NUL) | status |
| $34 Write chunk | length 1..128 at 0, bytes at 1 | status |
| $35 Commit write | none | status |
| $36 Cancel write | none | status |
| $37 Storage info | none | total bytes u32 at 2, free bytes u32 at 6 |
| $38 Copy file | selected index at 0 | remembers canonical source in caller-owned dormant clipboard |
| $39 Paste file | none | streams copied file into current directory; preserves existing targets |
| $3A Delete file | selected index at 0 | unlinks selected file after application confirmation |
| $50 Decode image | selected index at 0 | width/height/stride at 2/3/4, bitmap bytes u16 at 5; error string at 7 |
| $51 Bitmap chunk | bitmap offset u16 at 0, count 1..128 at 2 | returned count at 2, bytes at 3 |
| $52 Release library | none | unloads code/BSS/workspace, preserves clipboard and restores OS |
| $53 Library capabilities | none | A8L1 at 2; format mask at 6; max width/height at 7/8; file tools at 9; firmware string at 10; library file present at 42; APP file presence mask at 43 |
| $3F Return to menu | none | shared caller restores stock/alternative menu; app cold-starts it |

Enumeration includes documents as well as executable images, retains directory
sorting, names/indices, hidden/system filtering and the 255-entry bound. The
legacy command $01 still exposes only its existing runnable formats. Generic
read/metadata checks indices, paths, directories and lengths before touching I/O.

Writes use a same-directory temporary file, bounded streaming chunks, exact byte
counts, close/sync, and a recoverable replacement sequence. Existing targets are
not opened with truncate. If replacement fails, restore the original or preserve
its named backup for recovery. Disk-full, short write and cancel retain originals.
Internal transaction files are excluded from application enumeration.

The return command restores normal stock/alternative menu data without calling
HOS auto-start again. It must not require another resident 8 KB SRAM bank; stock
menu source remains immutable in flash. Missing alternative menus select stock.

Atari SIO drives are handled by application code through the OS SIO vector. They
do not pretend to be USB or cartridge flash. Initial filesystem support is DOS 2
single-density: real sector reads, directory browsing and bounded document I/O.
Offline drives and unsupported disk formats produce visible errors.

Check $53 before new file/library commands; older compatible firmware lacks
these optional features. $50 resolves the file through existing metadata and
loads `/SYS/IMAGE.LIB` (falling back to `/IMAGE.LIB` for existing installs), then executes its relocated A8L1 entry on the fixed worker
stack. Retrieve the complete bitmap before any directory enumeration; $52
releases the shared workspace and enumeration restores directory records.
The five presence-mask bits are Notepad, Calculator, Snake, Paint and Viewer;
these are file-presence diagnostics, not a claim of compatible/valid app bodies.
Delete/copy apply to files, not recursive folders. Clipboard paste supports
32-bit file sizes independently of the 16-bit document write ABI.
