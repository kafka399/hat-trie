#include <iostream>
#include <string>

#include "hat-trie.h"

using namespace std;
using namespace vaszauskas;

int indexof(char ch) {
    if (ch - 'a' < 26) {
        return ch - 'a';
    }
    return -1;
}

int main() {
    hat_trie<26, indexof> ht;

    ht.insert("the");
    ht.insert("their");
    ht.insert("c");
    ht.insert("dah");
    ht.insert("ehh");

    cout << ht.search("the") << endl;
    cout << ht.search("b") << endl;
    cout << ht.search("c") << endl;
    cout << ht.search("dah") << endl;
    cout << ht.search("e") << endl;
    cout << ht.search("f") << endl;

    return 0;
}

