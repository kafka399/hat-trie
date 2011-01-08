#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "hat-trie.h"

using namespace std;
using namespace vaszauskas;

int indexof(char ch) {
    if (ch - 'a' < 26 && ch - 'a' >= 0) {
        return ch - 'a';
    }
    return -1;
}

string trim(string s) {
    string result;
    for (size_t i = 0; i < s.length(); ++i) {
        s[i] = tolower(s[i]);
        if (indexof(s[i]) != -1) {
            result += s[i];
        }
    }
    return result;
}

bool test(const string& s, hat_trie<26, indexof>& ht, const set<string>& st) {
    cout << "TESTING " << s << ": " << flush;
    bool a;
    try { a = ht.search(s); }
    catch (bad_index e) { a = false; }
    bool b = st.find(s) != st.end();
    cout << a << " " << b << endl;
    return a == b;
}

int main() {
    hat_trie<26, indexof> ht;
    vector<string> v;
    set<string> s;

    string reader;
    while (cin >> reader) {
        v.push_back(reader);
        reader = trim(reader);
        if (reader.length() > 0) {
            ht.insert(reader);
            if (s.find(reader) == s.end()) {
                s.insert(reader);
                //ht.print(ht.root, ht.type);
            }
        }
    }
    cout << "TRIE STRUCTURE:" << endl;
    ht.print(ht.root, ht.type);
    cout << endl;
    for (size_t i = 0; i < v.size(); ++i) {
        assert(test(v[i], ht, s));
    }
    test("earth.", ht, s);
    return 0;
}

