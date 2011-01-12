#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>  // for sleep()

#include "array-hash.h"
#include "hat-trie.h"

using namespace std;
using namespace stx;

int indexof(char ch) {
    if (ch - 'a' < 26 && ch - 'a' >= 0) {
        return ch - 'a';
    }
    if (ch == '\'') return 26;
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

bool test(const string& s, hat_trie<27, indexof>& ht, const set<string>& st) {
    cout << "TESTING " << s << ": " << flush;
    bool a;
    try { a = ht.search(s); }
    catch (bad_index e) { a = false; }
    bool b = st.find(s) != st.end();
    cout << a << " " << b << endl;
    return a == b;
}

void print(const pair<const char *, uint16_t>& p) {
    for (int i = 0; i < p.second; ++i) {
        cout << p.first[i];
    }
    cout << endl;
}

template <class T>
void print(T& t) {
    typename T::iterator it;
    for (it = t.begin(); it != t.end(); ++it) {
        cout << *it << endl;
    }
}

template <class A>
bool compare(const A& a, const set<string>& s) {
    set<string>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        if (a.find(it->c_str()) == false) {
            return false;
        }
    }
    return s.size() == a.size();
}

int main() {
    std::ios_base::sync_with_stdio(false);
    array_hash ah;
    string reader;
    set<string> s;
    while (cin >> reader) {
        ah.insert(reader.c_str(), reader.length());
        s.insert(reader);
    }
    assert(compare(ah, s));

    return 0;
}

