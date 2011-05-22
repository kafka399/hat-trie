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
#include <stdint.h>  // TODO generalize this to windows platforms
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

template <class T>
class array_hash { };

/**
 * Hash table container for unsorted strings.
 */
template <>
class array_hash<std::string> {

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

// ----------
// array_hash
// ----------

/**
 * Default constructor.
 */
array_hash<std::string>::array_hash(const array_hash_traits &aht) :
        _traits(aht), _size(0) {
    _data = new char *[_traits.slot_count];
    for (int i = 0; i < _traits.slot_count; ++i) {
        _data[i] = NULL;
    }
}

/**
 * Standard destructor.
 */
array_hash<std::string>::~array_hash() {
    for (int i = 0; i < _traits.slot_count; ++i) {
        delete [] _data[i];
    }
    delete [] _data;
}

/**
 * Inserts @a str into the table.
 *
 * @param str  string to insert
 * @return  true if @a str is successfully inserted, false if @a str already
 *          appears in the table
 */
bool
array_hash<std::string>::insert(const char *str) {
    length_type length;
    int slot = _hash(str, length);
    char *p = _data[slot];
    if (p) {
        size_type occupied;
        if (_search(str, p, length, occupied) != NULL) {
            // str is already in the table. Nothing needs to be done.
            return false;
        }

        // Resize the slot if it doesn't have enough space.
        size_type current = *((size_type *)(p));
        size_type required = occupied + sizeof(length_type) + length;
        if (required > current) {
            _grow_slot(slot, current, required);
        }

        // Position for writing to the slot.
        p = _data[slot] + occupied - sizeof(length_type);

    } else {
        // Make a new slot for this string.
        size_type required = sizeof(size_type) + 2 * sizeof(length_type) + length;
        _grow_slot(slot, 0, required);

        // Position for writing to the slot.
        p = _data[slot] + sizeof(size_type);
    }

    // Write str into the slot.
    _append_string(str, p, length);
    ++_size;
    return true;
}

/**
 * Erases a string from the hash table.
 *
 * @param str  string to erase
 */
void
array_hash<std::string>::erase(const char *str) {
    length_type length;
    int slot = _hash(str, length);
    char *p = _data[slot];
    if (p) {
        size_type occupied;
        if ((p = _search(str, p, length, occupied)) != NULL) {
            // Erase the old word by overwriting it.
            int n = _traits.allocation_chunk_size - (p - _data[slot]);
            memcpy(p, p + sizeof(length_type) + length, n);
            --_size;

            // If that made the slot empty, erase the slot.
            if (*((length_type *)(_data[slot] + sizeof(size_type))) == 0) {
                delete [] _data[slot];
                _data[slot] = NULL;
            }
        }
    }
}

/**
 * Searches for @a str in the table.
 *
 * @param str  string to search for
 * @return  true iff @a str is in the table
 */
bool array_hash<std::string>::contains(const char *str) const {
    // Determine which slot in the table should contain str.
    length_type length;
    char *p = _data[_hash(str, length)];

    // Return true if p is in that slot.
    if (p == NULL) {
        return false;
    }
    size_type s;
    return _search(str, p, length, s) != NULL;
}

/**
 * Searches for @a str in the table.
 *
 * @param str  string to search for
 * @return  iterator to @a str in the table, or @a end() if @a str
 *          is not in the table
 */
array_hash<std::string>::iterator
array_hash<std::string>::find(const char *str) const {
    // Determine which slot in the table should contain str.
    length_type length;
    int slot = _hash(str, length);
    char *p = _data[slot];

    // Search for str in that slot.
    if (p == NULL) {
        return end();
    }
    size_type s;
    p = _search(str, p, length, s);
    return iterator(slot, p, _data, *this);
}

/**
 * Gets the number of elements in the table.
 */
size_t
array_hash<std::string>::size() const {
    return _size;
}

/**
 * Gets an iterator to the first element in the table.
 */
array_hash<std::string>::iterator
array_hash<std::string>::begin() const {
    iterator result;
    if (size() == 0) {
        result = end();
    } else {
        result._data = _data;
        while (result._data[result._slot] == NULL) {
            ++result._slot;
        }
        result._p = result._data[result._slot] + sizeof(size_type);
    }
    result._slot_count = _traits.slot_count;
    return result;
}

/**
 * Gets an iterator to one past the last element in the hash table.
 */
array_hash<std::string>::iterator
array_hash<std::string>::end() const {
    return iterator(_traits.slot_count, NULL, _data, *this);
}

/**
 * Hashes @a str to an integer, its slot in the hash table.
 *
 * @param str     string to hash
 * @param length  length of @a str. Calculated as this function runs
 * @param seed    seed for the hash function
 *
 * @return  hashed value of @a str, its slot in the table
 */
int
array_hash<std::string>::_hash(const char *str, length_type& length, int seed) const {
    int h = seed;
    length = 0;
    while (str[length]) {
        // Hash this character.
        h = h ^ ((h << 5) + (h >> 2) + str[length]);
        ++length;
    }

    ++length;  // include space for the NULL terminator
    return h & (_traits.slot_count - 1);  // same as h % _traits.slot_count
                                          // if _traits.slot_count is a
                                          // power of 2
}

/**
 * Searches for @a str in the table.
 *
 * @param str       string to search for
 * @param length    length of @a str
 * @param p         slot in @a data that @a str goes into
 * @param occupied  number of bytes in the slot that are currently in use.
 *                  This value is only meaningful when this function doesn't
 *                  find @a str
 *
 * @return  If @a str is found in the table, returns a pointer to the string
 *          and its corresponding length. If not, returns NULL.
 */
char *
array_hash<std::string>::_search(
        const char *str,
        char *p,
        length_type length,
        size_type &occupied) const {
    occupied = -1;
    char *start = p;

    // Search for str in the slot p points to.
    p += sizeof(size_type);  // skip past size at beginning of slot
    length_type w = *((length_type *) p);
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
        w = *((length_type *) p);
    }
    occupied = p - start + sizeof(length_type);
    return NULL;
}

/**
 * Increases the capacity of a slot to be >= required.
 *
 * @param slot      slot to change
 * @param current   current size of the slot
 * @param required  required size of the slot
 */
void
array_hash<std::string>::_grow_slot(int slot, size_type current, size_type required) {
    // Determine how much space the new slot needs.
    size_type new_size = current;
    while (new_size < required) {
        new_size += _traits.allocation_chunk_size;
    }

    // Make a new slot and copy all the data over.
    char *p = _data[slot];
    _data[slot] = new char[new_size];
    if (p != NULL) {
        memcpy(_data[slot], p, current);
        delete [] p;
    }
    *((size_type *)(_data[slot])) = new_size;
}

/**
 * Appends a string to a list of strings in a slot.
 *
 * @param str     string to append
 * @param p       pointer to the location in the slot this string
 *                should occupy
 * @param length  length of @a str
 */
void
array_hash<std::string>::_append_string(const char *str, char *p, length_type length) {
    // Write the length of the string, the string itself, the NULL
    // terminator, and a 0 after all of that (for the length of the
    // next string).
    memcpy(p, &length, sizeof(length_type));
    p += sizeof(length_type);
    memcpy(p, str, length);
    p += length;
    length = 0;
    memcpy(p, &length, sizeof(length_type));
}

// --------------------
// array_hash::iterator
// --------------------

/**
 * Standard default constructor.
 */
array_hash<std::string>::iterator::iterator() :
        _slot(0), _p(NULL), _data(NULL) {

}

/**
 * Standard copy constructor.
 */
array_hash<std::string>::iterator::iterator(const iterator& rhs) {
    *this = rhs;
}

/**
 * Move this iterator forward to the next element in the table.
 *
 * @return  self-reference
 */
array_hash<std::string>::iterator&
array_hash<std::string>::iterator::operator++() {
    // Move p to the next string in this slot.
    if (_p) {
        _p += *((length_type *) _p) + sizeof(length_type);
        if (*((length_type *) _p) == 0) {
            // Move down to the next slot.
            ++_slot;
            while (_slot < _slot_count && _data[_slot] == NULL) {
                ++_slot;
            }

            if (_slot == _slot_count) {
                // We are at the end. Make this an end iterator
                _p = NULL;
            } else {
                // Move to the first element in this slot.
                _p = _data[_slot] + sizeof(size_type);
            }
        }
    }
    return *this;
}

/**
 * Move this iterator backward to the previous element in the table.
 *
 * @return  self-reference
 */
array_hash<std::string>::iterator&
array_hash<std::string>::iterator::operator--() {
    // TODO
    return *this;
}

/**
 * Iterator dereference operator.
 *
 * @return  character pointer to the string this iterator points to
 */
const char *
array_hash<std::string>::iterator::operator*() const {
    if (_p) {
        return _p + sizeof(length_type);
    }
    return NULL;
}

/**
 * Standard equality operator.
 */
bool
array_hash<std::string>::iterator::operator==(const iterator& rhs) {
    return _p == rhs._p;
}

/**
 * Standard inequality operator.
 */
bool
array_hash<std::string>::iterator::operator!=(const iterator& rhs) {
    return !operator==(rhs);
}

}  // namespace stx

#endif  // ARRAY_HASH_H

