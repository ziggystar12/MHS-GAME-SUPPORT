"""Build an MHS EasyFlash ROM bank diagnostic (no game data)."""
from pathlib import Path
import argparse
import struct


def make_crt(megabytes):
    banks = megabytes * 64
    header = bytearray(64)
    header[:16] = b"C64 CARTRIDGE   "
    struct.pack_into(">IHHBB", header, 16, 64, 0x0100, 32, 1, 0)
    title = f"MHS EasyFlash {megabytes}MB test".encode("ascii")
    header[32:32 + len(title)] = title
    image = bytearray(header)
    for bank in range(banks):
        for address, half in ((0x8000, 0), (0xa000, 1)):
            rom = bytearray([0xff]) * 8192
            rom[:4] = bytes((bank, half, bank ^ 0xff, 0x4d))
            if bank == 0 and half == 1:
                # Reset in Ultimax, then idle in C64 RAM during bank checks.
                rom[0x100:0x11e] = bytes.fromhex(
                    "78 a2 ff 9a a9 2f 85 00 a9 37 85 01 "
                    "a9 4c 8d 00 02 a9 00 8d 01 02 a9 02 8d 02 02 4c 00 02")
                rom[-6:] = bytes.fromhex("00 e1 00 e1 00 e1")
            image.extend(struct.pack(">4sIHHHH", b"CHIP", 8208, 0, bank, address, 8192))
            image.extend(rom)
    return bytes(image)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("megabytes", type=int, choices=(1, 2, 4))
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(make_crt(args.megabytes))
    print(f"Wrote {args.megabytes}MB EasyFlash test: {args.output}")
