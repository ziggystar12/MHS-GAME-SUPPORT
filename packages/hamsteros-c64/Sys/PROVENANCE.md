# HamsterOS_C64 spelling data

`SPELL.DAT` is an exact copy of
`E:/MHS-Repository/HamsterOS/contrib/extras/SPELL.DAT`, copied on September 20,
2026. It was not regenerated, reduced, or modified for the C64 edition.

- SHA-256: `e72efc0bc32978712849f0b92f1b78aa3837c060e28661341f1a2c8e4821a984`
- Size: 217,136 bytes.
- Format: HamsterWrite HSPD v2, US English.
- Vocabulary: 107,194 words represented by a 196,608-byte Bloom filter.
- Correction lexicon: 2,309 words in 20,480 bytes.

The source is the English Speller Database (ESDB, formerly SCOWL) US English
size-60 word list, with corrections selected from the size-35 word list.
HamsterOS records source revision `en-wl/wordlist`
`1e5b7d3a72f47a71da5d28686c1dd4b397178485` (v2 branch, July 22, 2026) in
`contrib/extras/SPELL_SOURCE.md`.

`ESDB-COPYRIGHT.txt` is copied verbatim from
`E:/MHS-Repository/HamsterOS/third_party/esdb/COPYRIGHT.txt`. Redistribute that
notice with the dictionary. `SPELL_REPORT.txt` is the original generation report.
Its 0.1132% false-positive figure is a Bloom-filter estimate, not a measured
guarantee of spelling accuracy.

`src/hos_spell.c` retains the lookup and suggestion algorithms from
`E:/MHS-Repository/HamsterOS/apps/hamwrite.c`: case-insensitive FNV-1a hashing,
two-hash Bloom probes, the original 177 common suggestion words in their original
order, and up to five Levenshtein-distance suggestions with distance at most
three. The original source comment calls that common list "200" words; its
actual entry count is 177. The generator is HamsterOS `tools/mkspell.py`.

The C64 implementation uses a 48 KiB page cache and resumable jobs instead of
loading the entire dictionary. It deliberately reports unavailable or I/O error
when the dictionary cannot be read. It does not mark unknown words as misspelled
using only the small common vocabulary. An optional callback supplies custom
dictionary and Ignore All membership; the host owns their persistence.

This service covers ASCII English spelling with HamsterWrite's existing
short-word, long-word, all-caps and number exceptions. It does not provide grammar
checking, Unicode language support or exact dictionary membership. Suggestion
ordering is preserved for compatibility and is not frequency-ranked.
