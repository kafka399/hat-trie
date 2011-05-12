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

// TODO backwards iteration

#ifndef ARRAY_HASH_H
#define ARRAY_HASH_H

#include <cstring>
#include <cstdint>  // TODO generalize this to windows platforms
#include <utility>
#include <iterator>

namespace stx {

/**
 * Provides a way to tune the performance characteristics of an
 * array hash table.
 */
class array_hash_traits {

  public:

    array_hash_traits() {
        slot_count = 512;
        allocation_chunk_size = 32;
    }

    /// number of slots in the hash table
    int slot_count;

    /// when a slot in the array hash is allocated,
    int allocation_chunk_size;

};

/**
 * Hash table container for unsorted strings.
 */
class array_hash {

  private:
    typedef uint16_t length_type;
    typedef uint32_t size_type;

  public:
    class iterator;

    array_hash(const array_hash_traits & = array_hash_traits());
    ~array_hash();

    // accessors
    bool contains(const char *str) const;
    iterator find(const char *str) const;
    size_t size() const;

    // modifiers
    bool insert(const char *str);
    void erase(const char *str);

    iterator begin() const;
    iterator end() const;

    class iterator : std::iterator<std::bidirectional_iterator_tag,
                                   const char *> {
        friend class array_hash;

      public:
        iterator();
        iterator(const iterator& rhs);

        iterator& operator++();
        iterator& operator--();

        const char *operator*() const;
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);

      private:
        int _slot;
        char *_p;
        char **_data;
        int _slot_count;

        iterator(int slot, char *p, char **data,
                 const array_hash &owner = array_hash()) :
                 _slot(slot), _p(p), _data(data),
                 _slot_count(owner._traits.slot_count) { }
    };

  private:
    array_hash_traits _traits;
    size_t _size;
    char **_data;

    int _hash(const char *str, length_type &length, int seed = 23) const;
    char *_search(const char *, char *, length_type, size_type &) const;
    void _grow_slot(int slot, size_type current, size_type required);
    void _append_string(const char *str, char *p, length_type length);
};

} // namespace stx

#endif  // ARRAY_HASH_H

