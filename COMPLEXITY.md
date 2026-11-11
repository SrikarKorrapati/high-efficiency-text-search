# Time complexity: before and after

**Before** means a straightforward search that checks every pattern at every possible position. **After** means the Rabin–Karp and Bloom filter implementation in `main.cpp`. The repository had no earlier implementation; the comparison is theoretical, not a benchmark.

## Terms

- `n`: number of bytes in the text.
- `k`: number of patterns.
- `S`: total number of bytes across all patterns.
- `d`: number of distinct pattern lengths.
- `C`: work spent comparing candidate patterns with the actual text.
- `Z`: total number of matches stored, including duplicates.

## Before: straightforward search

For a pattern of length `m`, check up to `n - m + 1` positions and compare up to `m` bytes at each position.

- One pattern: `O(n * m)` worst-case search time.
- All patterns: `O(n * S)` worst-case search time.
- If all `k` patterns have length `m`: `O(n * k * m)`.

Long matching prefixes or many full matches can cause this worst case. Reading input and storing results add `O(n + S + Z)` work.

## After: Rabin–Karp with a Bloom filter

| Part | Time |
| --- | --- |
| Group patterns by length using `map` | `O(k * log(d + 1))` |
| Hash patterns and prepare rolling hashes | `O(S)` |
| Insert hashes into the filter and hash table | `O(k)` expected |
| Scan the text once per usable pattern length | `O(n * d)` expected |
| Verify candidate matches against the actual text | `O(C)` |
| Store matches | `O(Z)` amortized |

Including input, the expected total is:

```text
O(n + S + k * log(d + 1) + n * d + C + Z)
```

This assumes the expected constant-time operations of `unordered_map`. The three separate polynomial hashes use different bases and moduli; updating all three rolling hashes and mixing their values for the three Bloom filter probes takes constant time per window. This adds a constant amount of hashing work compared with maintaining one hash, without changing the asymptotic bounds. Patterns longer than the text are skipped.

The Bloom filter has a fixed 100,003 bits. Clearing it once per group is treated as constant work per group above. If its size were a variable `B`, initialization would add up to `O(d * B)` bit work.

If all patterns have the same length `m`, the comparison becomes:

```text
Before: O(n * k * m) worst-case search work
After:  O(k * m + n + C + Z) expected work
```

For example, 100 patterns of length 10 can require about 1,000 byte comparisons per position in the straightforward approach. This program shares one rolling-hash scan across those patterns and compares only candidates. This explains the possible saving; it is not a measured speedup.

## What the Bloom filter changes

The filter rejects windows whose hashes are definitely absent. A positive result still needs a hash-table lookup and, if the hash exists, exact text comparisons.

It can reduce hash-table lookups, but it does **not** improve the asymptotic complexity over grouped Rabin–Karp with a hash table alone. Its extra checks may cost more than they save on small inputs. As the fixed filter fills up, it rejects fewer windows.

## Worst case

Exact verification can still take `O(n * S)` work. Repeated characters with many matching patterns are one example. Rabin–Karp hash collisions also cause extra comparisons. Bloom filter false positives alone only cause extra hash-table lookups.

With expected constant-time hash-table operations, the bound when verification is worst-case is:

```text
O(n + S + k * log(d + 1) + n * S + Z)
```

`unordered_map` also has linear worst-case operations. Pathological hash-table collisions can add up to `O(k * k)` construction work. Lookup work can reach `O(n * k)`, already covered by `O(n * S)` because patterns are nonempty. Thus the strict worst-case bound includes the extra `k * k` term.

## Space

Including the text, patterns, groups, hash buckets, filter, and stored matches:

```text
O(n + S + k + Z + B)
```

`B` is the filter storage: 100,003 bits, roughly 12.5 KB. Only one filter is used at a time. The initial window substring fits within the text-sized term. Storing all positions can dominate memory use when there are many matches.

These bounds count storing or reporting one position as one operation. Writing its decimal digits can take up to `O(log(n + 1))` characters per position.
