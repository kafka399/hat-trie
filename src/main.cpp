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

int main() {
    std::ios_base::sync_with_stdio(false);  // speed up reading from stdin
    hat_trie<> ht;
    string reader;
    set<string> s;
    ht_array_hash<> ah;

    ht.insert("");
    for (hat_trie<>::iterator it = ht.begin(); it != ht.end(); ++it) {
        cout << *it << endl;
    }
    return 0;

    while (cin >> reader) {
        ht.insert(reader);
        s.insert(reader);
    }

    hat_trie<>::const_iterator it;
    for (it = ht.begin(); it != ht.end(); ++it) {
        cout << *it << endl;
    }

    return 0;
}

