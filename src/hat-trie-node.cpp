#include "hat-trie-node.h"

namespace stx {

// ---------------------------------
// hat_trie_container implementation
// ---------------------------------

hat_trie_container::
hat_trie_container(char ch) :
        hat_trie_node_base(ch) {
    set_word(false);
}

bool hat_trie_container::
contains(const char *p) const {
    if (*p == '\0') {
        return is_word();
    }
    return store.contains(p);
}

bool hat_trie_container::
insert(const char *p) {
    if (*p == '\0') {
        bool b = is_word();
        set_word(true);
        return !b;
    }
    return store.insert(p);
}

// ----------------------------
// hat_trie_node implementation
// ----------------------------

hat_trie_node::
hat_trie_node(char ch) :
        hat_trie_node_base(ch) {
    for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
        children[i] = NULL;
    }
}

hat_trie_node::
~hat_trie_node() {
    for (int i = 0; i < HT_ALPHABET_SIZE; ++i) {
        delete children[i];
    }
}

}  // namespace stx

