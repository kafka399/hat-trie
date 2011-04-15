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
    vector<string> v;

    ht.insert("a");
    ht.insert("able");
    ht.insert("aaron");
    ht.insert("arg");
    ht.insert("the");
    ht.insert("there");
    ht.insert("torn");
    ht.insert("toward");
    ht.print();

    hat_trie<>::iterator it = ht.begin();
    for (int i = 0; i < 3; ++i) {
        cout << *it << endl << endl;
        ++it;
    }


    return 0;

    while (cin >> reader) {
        ht.insert(reader);
    }
	//ht.print();


    return 0;
}

