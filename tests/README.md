# Software checks

From the repository root:

```sh
python tools/check.py
python tools/check_mpe_host.py --release-dir /path/to/current/downloads
```

Checks cover current component hashes, package/engine headers and CRCs,
registration, retained source/rebuild records, firmware records and local links.
The optional release directory must contain Teensy.Support.Package.zip and
NESVM.zip. Checks bind both ZIP hashes, exact 11/14-member layouts and every
member to the replacement record and repository companions; no checksum or
source download is required. Add `--previous-release-dir /path/to/previous/downloads`
to prove that the member layouts and all other member bytes were preserved.
Damaged hosts are rejected. Software receipts bind exact files; physical
acceptance remains separate.

Retained interface/cache checks:

```sh
g++ -std=c++17 -O2 tests/c64-transfer-test.cpp -o .build/c64-transfer-test
gcc -std=gnu11 -O2 integration/kff2/mpe/tests/md16-test.c -o .build/md16-test
python tests/easyflash-format-test.py
```
