#include <cassert>
#include <random>

#define main textSearchMain
#include "../main.cpp"
#undef main

vector<vector<size_t>> directSearch(const string& text, const vector<string>& patterns) {
    vector<vector<size_t>> matches(patterns.size());
    for (size_t id = 0; id < patterns.size(); ++id) {
        if (patterns[id].size() > text.size()) {
            continue;
        }
        for (size_t start = 0; start <= text.size() - patterns[id].size(); ++start) {
            if (text.compare(start, patterns[id].size(), patterns[id]) == 0) {
                matches[id].push_back(start);
            }
        }
    }
    return matches;
}

void checkSearch(const string& text, const vector<string>& patterns) {
    assert(search(text, patterns) == directSearch(text, patterns));
}

void checkRollingHashes(const string& text) {
    for (size_t length = 1; length <= text.size(); ++length) {
        const HashValues powers = getPowers(length);
        HashValues hashes = getHashes(text.substr(0, length));
        for (size_t start = 1; start <= text.size() - length; ++start) {
            rollHashes(hashes, powers, text[start - 1], text[start + length - 1]);
            assert(hashes == getHashes(text.substr(start, length)));
        }
    }
}

int main() {
    const HashValues expected{6385249, 6435120, 6738420};
    assert(getHashes("ana") == expected);

    // Regression: the old adjacent-bit probes accepted "b" after adding "a" and "d".
    BloomFilter filter;
    filter.add(getHashes("a"));
    filter.add(getHashes("d"));
    assert(filter.mightContain(getHashes("a")));
    assert(filter.mightContain(getHashes("d")));
    assert(!filter.mightContain(getHashes("b")));

    checkSearch("bananas are bananas", {"ana", "bananas", "are", "apple"});
    checkSearch("aaaa", {"aa", "a", "aa", "aaaa", "aaaaa"});
    checkSearch("", {"a"});
    checkSearch("abc", {});
    checkSearch("one two one", {"one", " ", "two", "not there"});
    checkSearch("\xc3\xa9" "a" "\xc3\xa9", {"\xc3\xa9", "a", "\xc3\xa9" "a"});

    // Equal first hashes must still be distinguished by exact text comparison.
    const string zeros(4, '\0');
    const string collision("\x3b\x9a\xca\x07", 4);
    assert(getHashes(zeros)[0] == getHashes(collision)[0]);
    assert(getHashes(zeros) != getHashes(collision));
    checkSearch(zeros + "x" + collision, {zeros, collision, zeros});

    mt19937 random(42);
    for (int trial = 0; trial < 500; ++trial) {
        string text(random() % 65, '\0');
        for (char& ch : text) {
            // Exercise repeated text as well as arbitrary bytes, including NUL.
            ch = trial % 2 == 0 ? static_cast<char>('a' + random() % 4)
                                : static_cast<char>(random() % 256);
        }
        checkRollingHashes(text);

        vector<string> patterns;
        const size_t count = random() % 12;
        for (size_t id = 0; id < count; ++id) {
            string pattern(random() % 12 + 1, '\0');
            if (!text.empty() && id % 2 == 0) {
                pattern = text.substr(random() % text.size(), pattern.size());
            } else {
                for (char& ch : pattern) {
                    ch = static_cast<char>(random() % 256);
                }
            }
            patterns.push_back(pattern);
            filter.add(getHashes(pattern));
        }
        for (const string& pattern : patterns) {
            assert(filter.mightContain(getHashes(pattern)));
        }
        if (!patterns.empty()) {
            patterns.push_back(patterns.front());
        }
        checkSearch(text, patterns);
    }

    cout << "All search and rolling-hash checks passed.\n";
}
