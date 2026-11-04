# High-efficiency text search

A simple C++ program that finds multiple patterns in a line of text using Rabin–Karp and a Bloom filter.

Patterns are grouped by length. For each group, Rabin–Karp updates the hash as a window moves through the text. The Bloom filter skips hashes that cannot match any pattern in the group. Possible matches are looked up by hash and checked against the actual text, so hash collisions never produce incorrect results.

## Build and run

```sh
g++ -std=c++17 -O2 main.cpp -o text-search
./text-search
```

## Input

1. The text on one line.
2. The number of patterns.
3. One nonempty pattern per line.

Input is assumed to follow this format. Text and patterns can contain spaces, and patterns can have different lengths.

Example:

```text
bananas are bananas
4
ana
bananas
are
apple
```

Output:

```text
ana: 1 3 13 15
bananas: 0 12
are: 8
apple: not found
```

Positions start at zero and count bytes. Search is case-sensitive and includes overlapping matches. Results appear in input order, including duplicate patterns. An empty text or zero patterns is allowed.

The text is scanned once per distinct pattern length. Exact comparisons add work for candidate matches. The Bloom filter uses 100,003 bits and three probes; it can return false positives, which the later checks reject. It is an extra prefilter, and does not guarantee a speedup for every input.

See [COMPLEXITY.md](COMPLEXITY.md) for the time and space complexity before and after this approach.
