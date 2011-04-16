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

// TODO delete

#ifndef ht_array_hash_H
#define ht_array_hash_H

#include <cassert>
#include <iostream>
#include <stdint.h>
#include <cstring>
#include <utility>

#include "hat-trie-common.h"

namespace stx {

/**
 * Hash table container for unsorted strings.
 */
template <int alphabet_size = HT_DEFAULT_ALPHABET_SIZE,
          int (*indexof)(char) = ht_alphanumeric_index>
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

// ----------
// ht_array_hash
// ----------

/**
 * Default constructor.
 */
template <int alphabet_size, int (*indexof)(char)>
ht_array_hash<alphabet_size, indexof>::
ht_array_hash() {
    data = new char *[SLOT_COUNT];
    for (int i = 0; i < SLOT_COUNT; ++i) {
        data[i] = NULL;
    }
    _size = 0;
}

/**
 * Standard destructor.
 */
template <int alphabet_size, int (*indexof)(char)>
ht_array_hash<alphabet_size, indexof>::
~ht_array_hash() {
    for (int i = 0; i < SLOT_COUNT; ++i) {
        delete data[i];
    }
    delete data;
}

/**
 * Searches for @a str in the table.
 *
 * @param str     string to search for
 * @param length  length of @a str
 * @param p       slot in @a data that @a str goes into
 *
 * @return  If @a str is found in the table, returns a pointer to the string
 *          and its corresponding length. If not, returns NULL.
 */
template <int alphabet_size, int (*indexof)(char)>
char *ht_array_hash<alphabet_size, indexof>::
search(const char *str, length_type length, char *p) const {
    // Search for str in the slot p points to.
    p += sizeof(size_type);  // skip past size at beginning of slot
    length_type w = *((length_type *)p);
    while (w != 0) {
        p += sizeof(length_type);
        if (w == length) {
            // The string being scanned is the same length as str.
            // Make sure they aren't the same string.
            if (strncmp(str, p, length) == 0) {
                // Found str.
                return p - sizeof(length_type);
            }
        }
        p += w;
        w = *((length_type *)p);
    }
    return NULL;
}

/**
 * Inserts @a str into the table.
 *
 * @param str  string to insert
 *
 * @return  true if @a str is successfully inserted, false if @a str already
 *          appears in the table
 */
template <int alphabet_size, int (*indexof)(char)>
bool ht_array_hash<alphabet_size, indexof>::
insert(const char *str) {
    length_type length;
    int slot = hash(str, length);
    assert(length != 1);
    char *p = data[slot];
    if (p) {
        // Append the new string to the end of this slot.
        if (search(str, length, p) != NULL) {
            // str is already in the table. Nothing needs to be done.
            return false;
        }

        // Append the new string to the end of this slot.
        size_type old_size = *((size_type *)(p));
        size_type new_size = old_size + sizeof(length_type) + length;
        data[slot] = new char[new_size];
        memcpy(data[slot], p, old_size);
        *((size_type *)(data[slot])) = new_size;
        delete [] p;
        p = data[slot] + old_size - sizeof(length_type);

    } else {
        // Make a new slot for this string.
        size_type size = sizeof(size_type) + 2 * sizeof(length_type) + length;
        data[slot] = new char[size];
        *((size_type *)(data[slot])) = size;
        p = data[slot] + sizeof(size_type);
    }

    // Write data for s.
    memcpy(p, &length, sizeof(length_type));
    p += sizeof(length_type);
    memcpy(p, str, length);
    p += length;
    length = 0;
    memcpy(p, &length, sizeof(length_type));
    ++_size;

    // debug print code
//  for (size_t i = 0; i < *((size_type *)(data[slot])); ++i) {
//      cerr << int(data[slot][i]) << " ";
//  }
//  cerr << endl;
    return true;
}

/**
 * Searches for @a str in the table.
 *
 * @param str  string to search for
 * @return  true iff @a str is in the table
 */
template <int alphabet_size, int (*indexof)(char)>
bool ht_array_hash<alphabet_size, indexof>::
contains(const char *str) const {
    length_type length;
    char *p = data[hash(str, length)];
    if (p == NULL) {
        return false;
    }
    return search(str, length, p) != NULL;
}

/**
 * Searches for @a str in the table.
 *
 * @param str  string to search for
 * @return  iterator to @a str in the table, or @a end() if @a str
 *          is not in the table
 */
template <int alphabet_size, int (*indexof)(char)>
typename ht_array_hash<alphabet_size, indexof>::iterator
ht_array_hash<alphabet_size, indexof>::
find(const char *str) const {
    length_type length;
    int slot = hash(str, length);
    char *p = data[slot];
    if (p == NULL) {
        return end();
    }

    p = search(str, length, p);
    return iterator(slot, p, data);
}

/**
 * Gets the number of elements in the table.
 */
template <int alphabet_size, int (*indexof)(char)>
size_t ht_array_hash<alphabet_size, indexof>::
size() const {
    return _size;
}

/**
 * Gets an iterator to the first element in the table.
 */
template <int alphabet_size, int (*indexof)(char)>
typename ht_array_hash<alphabet_size, indexof>::iterator
ht_array_hash<alphabet_size, indexof>::
begin() const {
    iterator result;
    if (size() == 0) {
        result = end();
    } else {
        result.data = data;
        while (result.data[result.slot] == NULL) {
            ++result.slot;
        }
        result.p = result.data[result.slot] + sizeof(size_type);
    }
    return result;
}

/**
 * Gets an iterator to one past the last element in the hash table.
 */
template <int alphabet_size, int (*indexof)(char)>
typename ht_array_hash<alphabet_size, indexof>::iterator
ht_array_hash<alphabet_size, indexof>::
end() const {
    return iterator(SLOT_COUNT, NULL, data);
}

/**
 * Hashes @a str to an integer, its slot in the hash table.
 *
 * @param str     string to hash
 * @param length  length of @a str. This function calculates this value as it
 *                runs.
 * @param seed    seed for the hash function
 *
 * @return  hashed value of @a str, its slot in the table
 */
template <int alphabet_size, int (*indexof)(char)>
int ht_array_hash<alphabet_size, indexof>::
hash(const char *str, length_type& length, int seed) const {
    int h = seed;
    length = 0;
    while (str[length]) {
        // Make sure this character is a valid character. get_index
        // throws an exception if it is not.
        ht_get_index<alphabet_size, indexof>(str[length]);

        // Hash this character.
        h = h ^ ((h << 5) + (h >> 2) + str[length]);
        ++length;
    }

    ++length;  // include space for the NULL terminator
    return h & (SLOT_COUNT - 1);  // same as h % SLOT_COUNT if SLOT_COUNT
                                  // is a power of 2
}

// --------------------
// ht_array_hash::iterator
// --------------------

/**
 * Standard default constructor.
 */
template <int alphabet_size, int (*indexof)(char)>
ht_array_hash<alphabet_size, indexof>::
iterator::iterator() :
        slot(0), p(NULL), data(NULL) {

}

/**
 * Standard copy constructor.
 */
template <int alphabet_size, int (*indexof)(char)>
ht_array_hash<alphabet_size, indexof>::
iterator::iterator(const iterator& rhs) {
    *this = rhs;
}

/**
 * Move this iterator forward to the next element in the array hash.
 *
 * @return  self-reference
 */
template <int alphabet_size, int (*indexof)(char)>
typename ht_array_hash<alphabet_size, indexof>::iterator&
ht_array_hash<alphabet_size, indexof>::
iterator::operator++() {
    // Move p to the next string in this slot.
    if (p) {
        p += *((length_type *)p) + sizeof(length_type);
        if (*((length_type *)p) == 0) {
            // Move down to the next slot.
            ++slot;
            while (slot < SLOT_COUNT && data[slot] == NULL) {
                ++slot;
            }
            if (slot == SLOT_COUNT) {
                p = NULL;
            } else {
                p = data[slot] + sizeof(size_type);
            }
        }
    }
    return *this;
}

template <int alphabet_size, int (*indexof)(char)>
typename ht_array_hash<alphabet_size, indexof>::iterator&
ht_array_hash<alphabet_size, indexof>::
iterator::operator--() {
    // TODO
    return *this;
}

/**
 * Iterator dereference operator.
 *
 * @return  pair where @a first is a pointer to the (non-NULL terminated)
 *          string, and @a second is the length of the string
 */
template <int alphabet_size, int (*indexof)(char)>
const char *ht_array_hash<alphabet_size, indexof>::
iterator::operator*() const {
    if (p) {
        return p + sizeof(length_type);
    }
    return NULL;
}

/**
 * Standard equality operator.
 */
template <int alphabet_size, int (*indexof)(char)>
bool ht_array_hash<alphabet_size, indexof>::
iterator::operator==(const iterator& rhs) {
    return p == rhs.p;
}

/**
 * Standard inequality operator.
 */
template <int alphabet_size, int (*indexof)(char)>
bool ht_array_hash<alphabet_size, indexof>::
iterator::operator!=(const iterator& rhs) {
    return !(*this == rhs);
}

}  // namespace stx

#endif

