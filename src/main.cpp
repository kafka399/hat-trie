#include <cassert>
#include <map>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <time.h>
#include <unistd.h>  // for sleep()
#include <iomanip>
#include "array-hash.h"
#include "hat-trie.h"

using namespace std;
using namespace stx;

template <class T>
void print(const T &t) {
    typename T::iterator it;
    for (it = t.begin(); it != t.end(); ++it) {
        cout << *it << endl;
    }
}

int main() {
    std::ios_base::sync_with_stdio(false);  // speed up reading from stdin
    hat_trie<> ht;
    string reader;
    set<string> s;
    ht_array_hash<> ah;

    //ht.insert("hello");
    //ht.insert("world");
    //ht.insert("the");
    //ht.insert("and");
    print(ht);
    cout << ht.empty() << endl;
    return 0;

    while (cin >> reader) {
        ht.insert(reader);
        s.insert(reader);
    }

    hat_trie<>::iterator it;
    for (it = ht.begin(); it != ht.end(); ++it) {
        cout << *(ht.find(*it)) << endl;
    }

    return 0;
}

