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

// TODO erase function
// TODO backwards iteration
// TODO allocate in block chunks rather than just enough

#ifndef HT_ARRAY_HASH_H
#define HT_ARRAY_HASH_H

#include <cstring>
#include <stdint.h>  // TODO generalize this to windows platforms
#include <utility>
#include <iterator>

namespace stx {

const int HT_ALPHABET_SIZE = 128;

/**
 * Hash table container for unsorted strings.
 */
class ht_array_hash {
  private:
    typedef uint16_t length_type;
    typedef uint32_t size_type;

  public:
    class iterator;

    ht_array_hash();
    ~ht_array_hash();

    // accessors
    bool contains(const char *str) const;
    iterator find(const char *str) const;
    size_t size() const;

    // modifiers
    bool insert(const char *str);

    iterator begin() const;
    iterator end() const;

    class iterator : std::iterator<std::bidirectional_iterator_tag, const char *> {
        friend class ht_array_hash;

      public:
        iterator();
        iterator(const iterator& rhs);

        iterator& operator++();
        iterator& operator--();

        const char *operator*() const;
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);

      private:
        int slot;
        char *p;
        char **data;

        iterator(int slot, char *p, char **data) :
                slot(slot), p(p), data(data) { }
    };

  private:
    enum { SLOT_COUNT = 512 };  // MUST be a power of 2
    uint16_t _size;  // size can be no larger than 32768
    char **data;

    int hash(const char *str, length_type& length, int seed = 23) const;
    char *search(const char *str, length_type length, char *p) const;
};

} // namespace stx

#endif  // HT_ARRAY_HASH_H

