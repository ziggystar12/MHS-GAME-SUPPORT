#!/usr/bin/env python3
"""Check the shipped MPE host, registration and optional release downloads."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
import struct
import zipfile
import zlib
from pathlib import Path, PurePosixPath

ROOT = Path(__file__).resolve().parents[1]
CURRENT = json.loads((ROOT / 'MANIFEST.json').read_text())
HOST_SHA = CURRENT['hostSha256']
DESKTOP_SHA = CURRENT['desktopSha256']
sha = lambda data: hashlib.sha256(data).hexdigest()


def trh(data):
    assert len(data) >= 64, 'TRH header size'
    h = struct.unpack_from('<16I', data)
    assert h[:5] == (0x31485254, 1, 64, 0x60760000, 384 * 1024), 'TRH slot'
    zeroed = bytearray(data[:64]); struct.pack_into('<I', zeroed, 44, 0)
    assert zlib.crc32(zeroed) == h[11] and not any(h[12:]), 'TRH header CRC/reserved'
    assert 0x2000 <= h[5] <= h[4] and len(data) == 64 + h[5], 'TRH payload size'
    payload = data[64:]
    assert zlib.crc32(payload) == h[10], 'TRH payload CRC'
    word = lambda at: struct.unpack_from('<I', payload, at)[0]
    assert word(0) == 0x42464346 and word(0x1000) == 0x432000d1, 'TRH boot magic'
    assert h[7] == word(0x1004) and h[7] & 1, 'TRH Thumb entry'
    assert h[3] + 0x1000 <= (h[7] & ~1) <= h[3] + 0x3000, 'TRH entry bounds'
    assert word(0x1020) == h[3] and word(0x1024) == h[6] == h[5], 'TRH boot size'
    assert word(0x800) == 0x3248564d, 'MVH2 descriptor'
    assert h[8] == word(0x804) == 2 and h[9] == word(0x808) == 32, 'MPE services'
    assert word(0x80c) == 76 and word(0x81c) <= 0x10000, 'MPE prefix/code floor'
    assert payload[0x810:0x81c].split(b'\0')[0] == b'MHS MPE', 'MPE name'


def manifest_files(read, names):
    manifest = json.loads(read('MANIFEST.json'))
    entries = manifest['files']
    assert len({e['path'] for e in entries}) == len(entries), 'Duplicate manifest path'
    assert set(names) == {'MANIFEST.json'} | {e['path'] for e in entries}, 'Manifest coverage'
    for e in entries:
        path = PurePosixPath(e['path'])
        assert not path.is_absolute() and '..' not in path.parts and '\\' not in e['path']
        data = read(e['path'])
        assert len(data) == e['bytes'] and sha(data) == e['sha256'], e['path']
    return manifest


def zip_files(path):
    with zipfile.ZipFile(path) as z:
        assert z.testzip() is None and len(z.namelist()) == len(set(z.namelist()))
        return {n: z.read(n) for n in z.namelist()}


def check_bundle():
    folder = ROOT / 'firmware/mpe-host'
    files = {p.relative_to(folder).as_posix(): p.read_bytes() for p in folder.rglob('*') if p.is_file()}
    manifest = manifest_files(files.__getitem__, files)
    data = files['MPE.TRH']; trh(data)
    verification = json.loads(files['VERIFICATION.json'])
    assert sha(data) == HOST_SHA == manifest['hostSha256'] == verification['hostSha256']
    assert verification['software']['passed'] and not verification['physicalAcceptance']
    assert verification['desktopSha256'] == DESKTOP_SHA
    assert manifest['testedFirmwareCommit'] == verification['testedFirmwareCommit']
    assert verification['software']['standaloneVms']['passed']
    for offset in (0, 12, 44, 64, 64 + 0x800, len(data) - 1):
        bad = bytearray(data); bad[offset] ^= 1
        try: trh(bytes(bad))
        except AssertionError: pass
        else: raise AssertionError(f'Corruption accepted at {offset}')
    # Repair CRCs to check descriptor/header disagreement independently.
    bad = bytearray(data); struct.pack_into('<I', bad, 64 + 0x808, 0)
    struct.pack_into('<I', bad, 40, zlib.crc32(bad[64:])); struct.pack_into('<I', bad, 44, 0)
    struct.pack_into('<I', bad, 44, zlib.crc32(bad[:64]))
    try: trh(bytes(bad))
    except AssertionError: pass
    else: raise AssertionError('Mismatched descriptor accepted')
    assert files['VMS/MPE/manifest.vmi'] == b'VM1\nMPE\nmpe\nengine.mvm\nclient.crt\nEND\n'
    module = files['VMS/MPE/engine.mvm']; h = struct.unpack_from('<16I', module)
    header = bytearray(module[:64]); struct.pack_into('<I', header, 44, 0)
    assert h[:3] == (0x314d564d, 2, 64) and h[9] == 32
    assert zlib.crc32(header) == h[11] and zlib.crc32(module[64:]) == h[10]
    assert len(module) == 64 + h[3] + h[4] and h[12] == 0
    client = files['VMS/MPE/client.crt']
    assert len(client) == 24688 and client[:16] == b'C64 CARTRIDGE   '
    assert struct.unpack_from('>H', client, 22)[0] == 32 and client[0x4070:0x4074] == b'VMH1'
    assert files['MPE-DEMO.MPE'][0x4070:0x4074] == b'MGC1'
    assert files['Notices/MHS-Prism-Plus-LICENSE.txt'] == (ROOT / 'licenses/LICENSE-PRISM-PLUS.txt').read_bytes()
    assert not any(n.startswith(('Games/', 'Saves/', 'VMS/HAMSTEROS/')) for n in files)
    print('MPE host: slot, CRCs, corruption rejection, registration, licenses and software binding passed')
    return files


def check_release(folder, bundle):
    checksums = (folder / 'SHA256SUMS.txt').read_text().splitlines()
    expected = {'MPE.TRH', 'MPE-Host.zip', 'HAMSTEROS.MPE', 'HamsterOS-C64.zip'}
    assert expected <= {line.split('  ')[1] for line in checksums}
    for line in checksums:
        digest, name = line.split('  '); assert sha((folder / name).read_bytes()) == digest, name
    assert (folder / 'MPE.TRH').read_bytes() == bundle['MPE.TRH']
    assert zip_files(folder / 'MPE-Host.zip') == bundle
    companion = zip_files(folder / 'HamsterOS-C64.zip')
    manifest = manifest_files(companion.__getitem__, companion)
    assert sha(companion['Sys/HAMSTEROS.MPE']) == DESKTOP_SHA == manifest['desktopSha256']
    assert (folder / 'HAMSTEROS.MPE').read_bytes() == companion['Sys/HAMSTEROS.MPE']
    assert manifest['hostSha256'] == HOST_SHA and not manifest['physicalAcceptance']
    assert {n for n in companion if n.startswith('APPS/')} == {f'APPS/{n}.APP' for n in ('HAMNET', 'HAMWRITE', 'IMAGE', 'PAINT', 'SID', 'ZIPZORK')}
    assert all(n.startswith(('APPS/', 'MPE/Notices/', 'Sys/')) or n in {'README.txt', 'MANIFEST.json'} for n in companion)
    for n in ('MPE/Notices/Source-offer.txt', 'Sys/Notices/ZipZork-source-offer.txt'):
        assert b'2 October 2029' in companion[n] and b'three years after the last distribution' in companion[n]
    print('MPE release: download hashes, both ZIP readbacks and exact desktop companion passed')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--release-dir', type=Path)
    args = parser.parse_args()
    bundle = check_bundle()
    if args.release_dir: check_release(args.release_dir, bundle)
