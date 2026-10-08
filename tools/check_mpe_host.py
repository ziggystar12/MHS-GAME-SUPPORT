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
    assert verification['ancestry']['standaloneVms']['passed']
    boot = verification['software']['manifestBoot']
    assert boot['passed'] and boot['hostSha256'] == HOST_SHA
    assert boot['checks'] == {'total': 164, 'accepted': 24, 'rejected': 140, 'vms': 4}
    assert not boot['guestExecution'] and not boot['physicalAcceptance']
    assert verification['software']['stockCore']['passed']
    assert verification['software']['stockCore']['upstream'] == verification['testedFirmwareCommit']
    tested_desktop = verification['software']['host']['desktopSha256']
    assert verification['software']['services']['desktopSha256'] == tested_desktop
    binding = verification['desktopBinding']
    assert binding['shippedDesktopSha256'] == DESKTOP_SHA
    assert binding['testedDesktopSha256'] == tested_desktop
    assert binding['shippedDesktopPreserved'] and not binding['combinedDesktopQualified']
    assert tested_desktop != DESKTOP_SHA, 'Preserved and tested desktop receipts must stay distinct'
    source = verification['software']['source']
    qualification = source['qualification']
    assert qualification['passed'] and qualification['sourceRebuild']['passed']
    assert qualification['host']['sha256'] == qualification['sourceRebuild']['hostSha256'] == HOST_SHA
    assert qualification['sourceRebuild']['host']['sha256'] == HOST_SHA
    assert source['inventorySha256'] == qualification['sourceInventory']['sha256']
    assert source['inventoryFileCount'] > 0 and source['retainedPrivately'] and source['availableUnderWrittenOffer']
    for receipt, path in ((verification['software']['host'], 'tests/verification.json'),
                          (verification['software']['services'], 'service-tests/verification.json')):
        retained = next(e for e in qualification['checks'] if e['path'] == path)
        assert receipt['passed'] and receipt['hostSha256'] == HOST_SHA
        assert receipt['retainedReceiptSha256'] == retained['sha256']
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


TEENSY_MEMBERS = {f'APPS/{n}.APP' for n in ('HAMNET', 'HAMWRITE', 'IMAGE', 'PAINT', 'SID', 'ZIPZORK')} | {
    'Firmware/TeensyROM+_0.8.0.15_full.hex', 'MPE.TRH', 'README.txt', 'Sys/HAMSTEROS.MPE', 'Sys/SPELL.DAT'} | {
    f'VMS/MPE/{n}' for n in ('engine.mvm', 'client.crt', 'manifest.vmi')}
NES_MEMBERS = {'NESVM.MPE', 'Source-offer.txt'} | {f'VMS/NESVM/{n}' for n in (
    'COMPONENTS.json', 'LICENSE-DISPLAY-COMPONENTS.txt', 'LICENSE-MHS.txt', 'LICENSE-Nofrendo.txt',
    'LICENSE-PRISM-PLUS.txt', 'MPE-README.txt', 'NOTICES.md', 'qualification.json', 'README.md',
    'ROMS/README.txt', 'SAVES/README.txt', 'version.json')}
CHANGED_MEMBERS = {'MPE.TRH', 'README.txt'}
ADDED_MEMBERS = {'Firmware/TeensyROM+_0.8.0.15_full.hex'} | {
    f'VMS/MPE/{n}' for n in ('engine.mvm', 'client.crt', 'manifest.vmi')}
REMOVED_MEMBERS = {'Firmware/TeensyROM+_0.8.0.13_full.hex'}


def check_release(folder, bundle, previous_folder=None):
    verification = json.loads(bundle['VERIFICATION.json'])
    refresh = verification['publicationRefresh']
    assert refresh['passed'] and refresh['privateSourceRetained'] and refresh['otherAssetsPreserved']
    assert not refresh['romsPackaged'] and not refresh['physicalAcceptance']
    assert refresh['hostSha256'] == HOST_SHA
    assert refresh['hostSourceSha256'] == verification['software']['source']['qualification']['sourceArchive']['sha256']
    assert len(refresh['assets']) == 1 and refresh['assets'][0]['name'] == 'Teensy.Support.Package.zip'
    record = refresh['assets'][0]
    data = (folder / record['name']).read_bytes()
    assert len(data) == record['bytes'] and sha(data) == record['sha256']
    teensy = zip_files(folder / record['name'])
    assert set(teensy) == TEENSY_MEMBERS and len(teensy) == 14
    entries = record['files']
    assert len(entries) == len({e['path'] for e in entries}) == len(teensy)
    assert {e['path'] for e in entries} == TEENSY_MEMBERS
    assert set(record['changedMembers']) == CHANGED_MEMBERS
    assert set(record['addedMembers']) == ADDED_MEMBERS
    assert set(record['removedMembers']) == REMOVED_MEMBERS
    assert record['existingLayoutPreserved'] and record['unrelatedMembersPreserved']
    for e in entries:
        assert len(teensy[e['path']]) == e['bytes'] and sha(teensy[e['path']]) == e['sha256'], e['path']
    if previous_folder:
        old_data = (previous_folder / record['name']).read_bytes()
        assert sha(old_data) == record['previousSha256'], 'Previous support ZIP hash'
        old = zip_files(previous_folder / record['name'])
        assert set(old) - set(teensy) == REMOVED_MEMBERS
        assert set(teensy) - set(old) == ADDED_MEMBERS
        assert {n for n in set(old) & set(teensy) if old[n] != teensy[n]} == CHANGED_MEMBERS
    assert teensy['MPE.TRH'] == bundle['MPE.TRH']; trh(teensy['MPE.TRH'])
    core = 'Firmware/TeensyROM+_0.8.0.15_full.hex'
    assert teensy[core] == (ROOT / 'firmware/teensyrom-plus/TeensyROM+_0.8.0.15_full.hex').read_bytes()
    assert sha(teensy[core]) == verification['software']['stockCore']['firmware']['sha256']
    for name in TEENSY_MEMBERS - {'MPE.TRH', 'README.txt', core}:
        source = ROOT / ('firmware/mpe-host' if name.startswith('VMS/') else 'packages/hamsteros-c64')
        assert teensy[name] == (source / name).read_bytes(), name
    assert sha(teensy['Sys/HAMSTEROS.MPE']) == DESKTOP_SHA
    assert bundle['Notices/Source-offer.txt'] in teensy['README.txt']
    for name in ('CRC32-MIT.txt', 'FNET-Apache-2.0.txt', 'FNET-NOTICE.txt', 'MHS-Prism-Plus-LICENSE.txt',
                 'NUFLIX-NOTICES.md', 'PJRC.txt', 'Prism-MIT.txt', 'SdFat-MIT.txt', 'TeensyROM-MIT.txt'):
        assert bundle[f'Notices/{name}'] in teensy['README.txt'], name
    unchanged = refresh['unchangedAssets']
    assert {e['name'] for e in unchanged} == {'NESVM.zip', 'KFF2-Support.zip', 'A8PicoCart-Support.zip', 'DOOMVM.zip'}
    for e in unchanged:
        candidate = folder / e['name']
        if candidate.exists():
            raw = candidate.read_bytes()
            assert len(raw) == e['size'] and 'sha256:' + sha(raw) == e['digest'], e['name']
    # NESVM is retained unchanged; its previous qualification stays historical.
    if (folder / 'NESVM.zip').exists():
        nes = zip_files(folder / 'NESVM.zip'); assert set(nes) == NES_MEMBERS
        for name in NES_MEMBERS:
            assert nes[name] == (ROOT / 'packages/c64' / name).read_bytes(), name
        package = verification['ancestry']['standaloneVms']['packages'][0]
        assert package['id'] == 'NESVM' and sha(nes['NESVM.MPE']) == package['sha256']
    print('MPE release: exact 14-member setup, stock core/host/registration, preserved desktop/apps/notices and other assets passed')
    if previous_folder: print('MPE refresh: previous ZIP hash and exact changed/added/removed members passed')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--release-dir', type=Path)
    parser.add_argument('--previous-release-dir', type=Path, help='Optionally compare the two ZIPs before this refresh')
    args = parser.parse_args()
    if args.previous_release_dir and not args.release_dir: parser.error('--previous-release-dir requires --release-dir')
    bundle = check_bundle()
    if args.release_dir: check_release(args.release_dir, bundle, args.previous_release_dir)
