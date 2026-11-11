#include <bitset>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

const long long BASE = 256;
const long long MOD = 1000000007;

class BloomFilter {
    static const int SIZE = 100003;
    // 100,003 bits require approximately 12.5 KB of bit storage.
    bitset<SIZE> bits;

public:
    void add(long long hash) {
        long long step = hash / SIZE + 1;
        for (int i = 0; i < 3; ++i) {
            bits[(hash + i * step) % SIZE] = true;
        }
    }

    bool mightContain(long long hash) const {
        long long step = hash / SIZE + 1;
        for (int i = 0; i < 3; ++i) {
            if (!bits[(hash + i * step) % SIZE]) {
                return false;
            }
        }
        return true;
    }
};

long long getHash(const string& s) {
    long long hash = 0;
    for (unsigned char ch : s) {
        hash = (hash * BASE + ch) % MOD;
    }
    return hash;
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
            long long hash = getHash(patterns[id]);
            filter.add(hash);
            buckets[hash].push_back(id);
        }

        long long power = 1;
        for (size_t i = 1; i < length; ++i) {
            power = (power * BASE) % MOD;
        }

        long long hash = getHash(text.substr(0, length));
        for (size_t start = 0; start <= text.size() - length; ++start) {
            if (filter.mightContain(hash)) {
                auto it = buckets.find(hash);
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
                hash = (hash - outgoing * power % MOD + MOD) % MOD;
                hash = (hash * BASE + incoming) % MOD;
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
