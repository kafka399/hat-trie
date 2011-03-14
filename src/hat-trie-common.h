#ifndef HAT_TRIE_COMMON_H
#define HAT_TRIE_COMMON_H

#include <exception>

using namespace std;

namespace stx {

/**
 * Exception class for unindexed characters.
 *
 * This exception is thrown when a hat_trie encounters an unindexed
 * character. indexof() returns a value out of range (less than 0 or
 * greater than @a alphabet_size) for unindexed characters.
 */
class unindexed_character : public exception {
    virtual const char *what() const throw() {
        return "hat_trie: found unindexed character.";
    }
};

/// default value for hat_trie alphabet size
const size_t HT_DEFAULT_ALPHABET_SIZE = 26;

/// default indexof function for hat_tries
inline int ht_alphabet_index(char ch) {
    return ch - 'a';
}

template <int alphabet_size, int (*indexof)(char)>
int ht_get_index(char ch) throw(unindexed_character) {
    int result = indexof(ch);
    if (result < 0 || result >= alphabet_size) {
        throw unindexed_character();
    }
    return result;
}

}

#endif

