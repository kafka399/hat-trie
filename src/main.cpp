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

    ht.insert("a");
    ht.insert("b");
    ht.insert("c");
    ht.insert("d");
    ht.insert("e");

    ht.insert("a");
    ht.insert("b");
    ht.insert("c");
    ht.insert("d");
    ht.insert("e");


    return 0;
}

