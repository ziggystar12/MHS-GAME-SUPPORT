"""Check diagnostic CRT geometry and every bank marker."""
from pathlib import Path
import importlib.util
import struct

source = Path(__file__).parents[1] / "tools/make-easyflash-test.py"
spec = importlib.util.spec_from_file_location("easyflash_test", source)
generator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)
for megabytes in (1, 2, 4):
    image = generator.make_crt(megabytes)
    banks = megabytes * 64
    assert image[:16] == b"C64 CARTRIDGE   "
    assert struct.unpack_from(">H", image, 22)[0] == 32
    assert len(image) == 64 + banks * 2 * 8208
    for bank in range(banks):
        for half in (0, 1):
            pos = 64 + (bank * 2 + half) * 8208
            assert struct.unpack_from(">4sIHHHH", image, pos) == (
                b"CHIP", 8208, 0, bank, (0x8000, 0xa000)[half], 8192)
            assert image[pos + 16:pos + 20] == bytes((bank, half, bank ^ 0xff, 0x4d))
    print(f"PASS: {megabytes}MB, {banks} banks, both ROM halves")
