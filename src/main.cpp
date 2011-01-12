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

int main() {
    std::ios_base::sync_with_stdio(false);
    array_hash ah;
    string reader;
    set<string> s;
    while (cin >> reader) {
        ah.insert(reader.c_str(), reader.length());
        //s.insert(reader);
    }
    print(ah);
    //print(s);
    //sleep(10);

//  hat_trie<27, indexof> ht;
//  vector<string> v;
//  set<string> s;

//  string reader;
//  while (cin >> reader) {
//      //v.push_back(reader);
//      reader = trim(reader);
//      if (reader.length() > 0) {
//          //ht.insert(reader);
//          s.insert(reader);
//      }
//  }
    //cout << "TRIE STRUCTURE:" << endl;
    //ht.print(ht.root, ht.type);
    //cout << endl;
    //for (size_t i = 0; i < v.size(); ++i) {
        //assert(test(v[i], ht, s));
    //}
    //test("earth.", ht, s);
    //sleep(10);
    return 0;
}

