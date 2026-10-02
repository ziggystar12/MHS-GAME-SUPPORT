#!/usr/bin/env python3
"""Check the public downloads and local documentation without hardware."""
# SPDX-License-Identifier: MIT
import json
import re
import struct
import zipfile
import zlib
from pathlib import Path
from check_mpe_host import check_bundle

ROOT = Path(__file__).resolve().parents[1]

def crt(data, expected_type, expected_capacity):
    assert data[:16] == b'C64 CARTRIDGE   ', 'CRT signature'
    header_size, version, cart_type = struct.unpack_from('>IHH', data, 16)
    assert header_size == 64 and version == 0x0100 and cart_type == expected_type
    assert data[24:26] == (b'\x01\x00' if expected_type == 32 else b'\x00\x00'), 'CRT mode'
    offset, banks, capacity = 64, {}, 0
    while offset < len(data):
        assert offset + 16 <= len(data) and data[offset:offset+4] == b'CHIP'
        size, chip_type, bank, address, payload = struct.unpack_from('>IHHHH', data, offset+4)
        assert size == payload+16 and offset+size <= len(data) and chip_type in (0, 2)
        if expected_type == 32:
            assert 0 <= bank < 64 and address in (0x8000, 0xa000) and payload == 8192
        else:
            assert 0 <= bank < 128 and address == 0x8000 and payload == 16384
        assert (bank,address) not in banks
        banks[bank,address] = data[offset+16:offset+size]
        capacity += payload; offset += size
    assert capacity == expected_capacity and offset == len(data)
    assert set(banks) == ({(b,a) for b in range(64) for a in (0x8000,0xa000)} if expected_type == 32 else {(b,0x8000) for b in range(128)})
    assert banks[0,0x8000][4:9] == b'\xc3\xc2\xcd80', 'CBM80 boot signature'
    return banks

def mvm(data):
    h = list(struct.unpack_from('<16I', data))
    assert h[0] == 0x314d564d and h[1] == 2 and h[2] == 64
    header = bytearray(data[:64]); struct.pack_into('<I', header, 44, 0)
    assert zlib.crc32(header) == h[11]
    assert zlib.crc32(data[64:]) == h[10]
    ro = h[13] if h[12] == 1 else 0
    assert len(data) == 64+h[3]+h[4]+ro
    assert h[3] <= 98304 and h[4]+h[5] <= 196608
    assert h[6] & 1 and h[7] <= (h[6]&~1) < h[7]+h[3]

def car(data):
    magic,cart_type,checksum,reserved=struct.unpack_from('>4sIII',data)
    assert magic==b'CART' and cart_type==42 and reserved==0
    assert len(data)==16+1048576 and sum(data[16:])&0xffffffff==checksum

def uf2(data):
    assert len(data)%512 == 0
    blocks=set()
    for at in range(0,len(data),512):
        a,b,flags,address,size,block,count,family=struct.unpack_from('<8I',data,at)
        assert (a,b)==(0x0a324655,0x9e5d5157) and flags&0x2000 and family==0xe48bff56
        assert 0<size<=476 and 0x10000000<=address and address+size<=0x10100000
        assert block not in blocks and count==len(data)//512
        assert struct.unpack_from('<I',data,at+508)[0]==0x0ab16f30
        blocks.add(block)
    assert blocks==set(range(len(data)//512))

def intel_hex(path):
    eof=False; data_bytes=0
    for line in path.read_text().splitlines():
        assert line.startswith(':') and not eof
        record=bytes.fromhex(line[1:]); assert len(record)==record[0]+5 and sum(record)%256==0
        kind=record[3]
        if kind==0: data_bytes+=record[0]
        elif kind==1: assert record[0]==0; eof=True
        else: assert kind in (2,3,4,5)
    assert eof and data_bytes>500000

def malformed_rejected(check, data, changes):
    for offset,value in changes:
        bad=bytearray(data); bad[offset]=value
        try: check(bytes(bad))
        except (AssertionError,struct.error,IndexError): pass
        else: raise AssertionError(f'Malformed input accepted at {offset}')

def main():
    check_bundle()
    from check_current import check_current
    check_current(crt,uf2,intel_hex)
    print('Checks passed; no physical hardware acceptance is implied.')

if __name__=='__main__': main()
