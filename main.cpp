#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct HashParameters {
    long long base;
    long long modulus;
};

// Three separately computed polynomial hash functions.
constexpr array<HashParameters, 3> HASH_PARAMETERS{{
    {256, 1000000007},
    {257, 1000000009},
    {263, 998244353},
}};

using HashValues = array<long long, HASH_PARAMETERS.size()>;

class BloomFilter {
    static const int SIZE = 100003;
    // 100,003 bits require approximately 12.5 KB of bit storage.
    bitset<SIZE> bits;

    static size_t bitIndex(long long hash, size_t function) {
        // Mix each hash separately so short strings do not cluster in nearby bits.
        uint64_t value = static_cast<uint64_t>(hash)
            + 0x9e3779b97f4a7c15ULL * (function + 1);
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return (value ^ (value >> 31)) % SIZE;
    }

public:
    void add(const HashValues& hashes) {
        for (size_t i = 0; i < hashes.size(); ++i) {
            bits[bitIndex(hashes[i], i)] = true;
        }
    }

    bool mightContain(const HashValues& hashes) const {
        for (size_t i = 0; i < hashes.size(); ++i) {
            if (!bits[bitIndex(hashes[i], i)]) {
                return false;
            }
        }
        return true;
    }
};

HashValues getHashes(const string& s) {
    HashValues hashes{};
    for (unsigned char ch : s) {
        for (size_t i = 0; i < hashes.size(); ++i) {
            const auto& [base, modulus] = HASH_PARAMETERS[i];
            hashes[i] = (hashes[i] * base + ch) % modulus;
        }
    }
    return hashes;
}

HashValues getPowers(size_t length) {
    HashValues powers;
    powers.fill(1);
    for (size_t position = 1; position < length; ++position) {
        for (size_t i = 0; i < powers.size(); ++i) {
            const auto& [base, modulus] = HASH_PARAMETERS[i];
            powers[i] = (powers[i] * base) % modulus;
        }
    }
    return powers;
}

void rollHashes(HashValues& hashes, const HashValues& powers,
                unsigned char outgoing, unsigned char incoming) {
    for (size_t i = 0; i < hashes.size(); ++i) {
        const auto& [base, modulus] = HASH_PARAMETERS[i];
        hashes[i] = (hashes[i] - outgoing * powers[i] % modulus + modulus) % modulus;
        hashes[i] = (hashes[i] * base + incoming) % modulus;
    }
}

vector<vector<size_t>> search(const string& text, const vector<string>& patterns) {
    vector<vector<size_t>> matches(patterns.size());
    map<size_t, vector<size_t>> groups;

    for (size_t i = 0; i < patterns.size(); ++i) {
        groups[patterns[i].size()].push_back(i);
    }

    // Patterns of the same length share one pass through the text.
    for (const auto& [length, ids] : groups) {
        if (length > text.size()) {
            continue;
        }

        BloomFilter filter;
        unordered_map<long long, vector<size_t>> buckets;
        for (size_t id : ids) {
            HashValues hashes = getHashes(patterns[id]);
            filter.add(hashes);
            // The first hash selects candidates; exact comparisons confirm matches.
            buckets[hashes[0]].push_back(id);
        }

        const HashValues powers = getPowers(length);
        HashValues hashes = getHashes(text.substr(0, length));
        for (size_t start = 0; start <= text.size() - length; ++start) {
            if (filter.mightContain(hashes)) {
                auto it = buckets.find(hashes[0]);
                if (it != buckets.end()) {
                    for (size_t id : it->second) {
                        // Hashes can collide, so check the actual text too.
                        if (text.compare(start, length, patterns[id]) == 0) {
                            matches[id].push_back(start);
                        }
                    }
                }
            }

            if (start < text.size() - length) {
                unsigned char outgoing = text[start];
                unsigned char incoming = text[start + length];
                rollHashes(hashes, powers, outgoing, incoming);
            }
        }
    }

    return matches;
}

int main() {
    string text;
    getline(cin, text);

    int count;
    cin >> count;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<string> patterns(count);
    for (string& pattern : patterns) {
        getline(cin, pattern);
    }

    auto matches = search(text, patterns);
    for (size_t i = 0; i < patterns.size(); ++i) {
        cout << patterns[i] << ":";
        if (matches[i].empty()) {
            cout << " not found";
        } else {
            for (size_t position : matches[i]) {
                cout << ' ' << position;
            }
        }
        cout << '\n';
    }
    return 0;
}
