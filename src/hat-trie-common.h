/*
 * Copyright 2010-2011 Chris Vaszauskas and Tyler Richard
 *
 * This file is part of a HAT-trie implementation following the paper
 * entitled "HAT-trie: A Cache-concious Trie-based Data Structure for
 * Strings" by Nikolas Askitis and Ranjan Sinha.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HAT_TRIE_COMMON_H
#define HAT_TRIE_COMMON_H

#include <exception>

namespace stx {

/**
 * Exception class for unindexed characters.
 *
 * This exception is thrown when a hat_trie encounters an unindexed
 * character. indexof() returns a value out of range (less than 0 or
 * greater than @a alphabet_size) for unindexed characters.
 */
class unindexed_character : public std::exception {
    virtual const char *what() const throw() {
        return "hat_trie: found unindexed character.";
    }
};

/// default value for hat_trie alphabet size
const size_t HT_DEFAULT_ALPHABET_SIZE = 62;

/// default indexof function for hat_tries
inline int ht_alphanumeric_index(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 10;
    }
    return ch - 'a' + 36;
}

template <int alphabet_size, int (*indexof)(char)>
int ht_get_index(char ch) throw(unindexed_character) {
    int result = indexof(ch);
    if (result < 0 || result >= alphabet_size) {
        throw unindexed_character();
    }
    return result;
}

}  // namespace stx

#endif

