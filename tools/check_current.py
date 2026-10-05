# SPDX-License-Identifier: MIT
# Additional current-release checks, imported by check.py.
import hashlib,json,re,struct,zipfile,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
EXCLUDED=re.compile(r'(?:^|/)(?:GBVM|GGVM|AGIVM|APPLE2VM|AppleIIVM|DOSVM|7800VM)(?:[/._-]|$)',re.I)
def sha(b): return hashlib.sha256(b).hexdigest()
def current_files():
 m=json.loads((ROOT/'MANIFEST.json').read_text())
 assert m['publicVmIds']==['NESVM']
 assert set(m['releaseAssets'])=={'Teensy.Support.Package.zip','KFF2-Support.zip','A8PicoCart-Support.zip','NESVM.zip','DOOMVM.zip'}
 assert m['softwarePassed'] and not m['physicalAcceptance'] and not m['commercialGameDataBundled']
 for e in m['files']:
  p=Path(e['path']); assert not p.is_absolute() and '..' not in p.parts and '\\' not in e['path']
  assert not EXCLUDED.search(e['path']),f'VM outside public release scope: {e["path"]}'
  b=(ROOT/p).read_bytes(); assert len(b)==e['bytes'] and sha(b)==e['sha256'],e['path']
 return m

def mpe(b):
 d=0x4070; p=0x6070
 assert b[:16]==b'C64 CARTRIDGE   ' and b[d:d+4]==b'MGC1'
 h=bytearray(b[d:d+256]); expected=struct.unpack_from('<I',h,56)[0];struct.pack_into('<I',h,56,0)
 assert zlib.crc32(h)==expected
 v,header=struct.unpack_from('<HH',b,d+4)
 total=struct.unpack_from('<I',b,d+8)[0];count,table=struct.unpack_from('<II',b,d+20)
 primary,content=struct.unpack_from('<II',b,d+40);crc=struct.unpack_from('<I',b,d+32)[0]
 assert v==1 and header==256 and total==len(b) and table==p and 0<count<=32
 directory=b[p:p+count*128];assert len(directory)==count*128 and zlib.crc32(directory)==crc
 assert primary<count and content<count
 entries={}
 for i in range(count):
  row=directory[i*128:(i+1)*128]; name=row[:64].split(b'\0')[0].decode('ascii')
  flags,size,offset,stored,checksum=struct.unpack_from('<5I',row,64)
  assert name not in entries and 0<=offset<=len(b) and stored<=len(b)-offset
  if flags==0:
   data=b[offset:offset+stored];assert size==stored and zlib.crc32(data)==checksum;entries[name]=data
  else: assert flags==1
 engine=entries['engine.mvm']; h=list(struct.unpack_from('<16I',engine)); head=bytearray(engine[:64]);struct.pack_into('<I',head,44,0)
 assert h[:3]==[0x314d564d,2,64] and zlib.crc32(head)==h[11] and zlib.crc32(engine[64:])==h[10]
 return entries

def check_current(crt_check,uf2_check,hex_check):
 m=current_files()
 assert {p.name for p in (ROOT/'packages/c64').glob('*.MPE')}=={'NESVM.MPE'}
 for name in ('NESVM',):
  entries=mpe((ROOT/'packages/c64'/(name+'.MPE')).read_bytes());c=entries['VMBOOT.BIN']
  assert len(c)==128 and c[:4]==b'MVP1' and struct.unpack_from('<II',c,4)==(1,128)
  assert c[12:92].split(b'\0')[0]==('/VMS/'+name).encode() and not any(c[92:124])
  assert zlib.crc32(c[:124])==struct.unpack_from('<I',c,124)[0]
  assert 'targets/kff2/engine.kfm' in entries
 mpe((ROOT/'packages/doomvm/DOOMVM.MPE').read_bytes());mpe((ROOT/'packages/hamsteros-c64/Sys/HAMSTEROS.MPE').read_bytes())
 uf2_check((ROOT/'firmware/a8picocart/A8PicoCart-MPE.uf2').read_bytes())
 hex_check(ROOT/'firmware/teensyrom-plus/TeensyROM+_0.8.0.13_full.hex')
 for n,magic in [('HAMSTEROS.MPE',b'A8H2'),('DOOM.MPE',b'A8D1')]: assert (ROOT/'packages/a8picocart'/n).read_bytes()[:4]==magic
 k=(ROOT/'packages/hamsteros-kff2/Sys/HAMSTEROS.MPE').read_bytes();assert k[:4]==b'KFP1' and struct.unpack_from('<III',k,4)==(1,128,len(k)) and k[64:80].rstrip(b'\0')==b'HAMSTEROS'
 header=bytearray(k[:128]);expected=struct.unpack_from('<I',header,60)[0];struct.pack_into('<I',header,60,0);assert zlib.crc32(header)==expected
 for at in (20,32):
  offset,size,crc=struct.unpack_from('<III',k,at);assert offset+size<=len(k) and zlib.crc32(k[offset:offset+size])==crc
 at,total,expected=struct.unpack_from('<III',k,44);decoded=bytearray()
 while len(decoded)<total:
  crc=struct.unpack_from('<I',k,at)[0];size=min(4096,total-len(decoded));block=k[at+4:at+4+size];assert len(block)==size and zlib.crc32(block)==crc;decoded.extend(block);at+=4+size
 assert at==len(k) and zlib.crc32(decoded)==expected
 acceptance=json.loads((ROOT/'firmware/kff2/Sys/Notices/H24-ACCEPTANCE.json').read_text());assert sha((ROOT/'firmware/kff2/KungFuFlash_v2.H24.upd').read_bytes())==acceptance['updaterSha256']
 for p in ROOT.rglob('*'):
  if not p.is_file() or '.git' in p.relative_to(ROOT).parts or '.build' in p.relative_to(ROOT).parts:continue
  assert not EXCLUDED.search(p.relative_to(ROOT).as_posix()),f'VM outside public release scope: {p}'
 assert not list((ROOT/'source').glob('*.zip')), 'Source archives stay with the private build snapshots'
 for p in ROOT.rglob('*.md'):
  if any(n.lower() in {'.git','.build','notices'} for n in p.relative_to(ROOT).parts):continue
  for target in re.findall(r'\[[^\]]*\]\(([^)]+)\)',p.read_text(encoding='utf-8')):
   if re.match(r'(https?://|mailto:|#)',target):continue
   assert (p.parent/target.split('#')[0]).resolve().exists(),f'Broken link: {p}: {target}'
 print('Public VM allowlist, component hashes, NESVM, Doom, three HamsterOS desktops, firmware, source and links passed')
