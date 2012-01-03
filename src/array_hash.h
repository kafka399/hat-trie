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

#ifndef ARRAY_HASH_H
#define ARRAY_HASH_H

#include <cstring>
#include <stdint.h>
#include <utility>
#include <iterator>

namespace stx {

/**
 * @brief Provides a way to tune the performance characteristics of an
 * array hash table.
 *
 * @subsection Usage
 * @code
 * array_hash_traits traits;
 * traits.slot_count = 256;
 * traits.allocation_chunk_size = 64;
 * hat_set<string> rawr(traits);
 * rawr.insert(...);
 * ...
 * @endcode
 */
class array_hash_traits
{
public:
    array_hash_traits(int slot_count = 512, int allocation_chunk_size = 32) :
        slot_count(slot_count), allocation_chunk_size(allocation_chunk_size)
    {
    }

    /**
     * Number of slots in the hash table. Higher values use more
     * memory but may show faster access times.
     *
     * Default 512. Must be a positive power of 2.
     */
    int slot_count;

    /**
     * This value only affects the speed of the insert() operation.  When a
     * slot in the array hash is allocated, it is allocated in blocks of this
     * size until there is enough space for a word.  In general, higher values
     * use more memory but require fewer memory copy operations.  Try to guess
     * how many average characters your strings will use, then multiply that
     * by (hat_trie_traits.burst_threshold / array_hash_traits::slot_count) to
     * get a good estimate for this value.
     *
     * If you want memory allocations to be exactly as big as they need to
     * be (rather than in block chunks), set this value to 0. This may be
     * your best option in memory-tight situations. Insertion speed is still
     * impressive even with abundant memory copy operations.
     *
     * Default 32. Must be non-negative.
     */
    int allocation_chunk_size;
};

template <class T>
class array_hash;

/// Time- and space-efficient hash table for strings
template <>
class array_hash<std::string>
{
  private:
    typedef uint16_t length_type;
    typedef uint32_t size_type;

  public:
    class iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef iterator const_iterator;
    typedef reverse_iterator const_reverse_iterator;

    /**
     * Default constructor.
     *
     * O(1)
     *
     * @param traits  array hash customization traits
     */
    array_hash(const array_hash_traits &traits = array_hash_traits()) :
            _traits(traits)
    {
        _init();
    }

    /**
     * Iterator range constructor.
     *
     * O(n) where n is the number of elements between first and last
     */
    template <class Iterator>
    array_hash(Iterator first, const Iterator& last,
            const array_hash_traits& traits = array_hash_traits()) :
            _traits(traits)
    {
        _init();

        // Insert the data in the iterator range
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    /**
     * Standard destructor.
     */
    ~array_hash()
    {
        _destroy();
    }

    /**
     * Copy constructor.
     *
     * O(n) where n = traits.slot_count
     */
    array_hash(const array_hash<std::string> &rhs)
    {
        _data = NULL;
        operator=(rhs);
    }

    /**
     * Assignment operator.
     *
     * O(n) where n = traits.slot_count
     */
    array_hash<std::string>& operator=(const array_hash<std::string> &rhs)
    {
        if (this != &rhs) {
            _traits = rhs._traits;
            _size = rhs._size;

            // Empty the current data array
            if (_data) {
                _destroy();
            }

            // Copy the data from the other array hash
            _data = new char *[_traits.slot_count];
            for (int i = 0; i < _traits.slot_count; ++i) {
                if (rhs._data[i]) {
                    size_t space = *rhs._data[i];
                    _data[i] = new char[space];
                    memcpy(_data[i], rhs._data[i], space);
                } else {
                    _data[i] = NULL;
                }
            }
        }
        return *this;
    }

    /**
     * Determines whether @a str is in the table.
     *
     * O(m) where m is the length of @a str
     *
     * @param str  string to search for
     * @return  true iff @a str is in the table
     */
    bool exists(const char *str) const
    {
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
     * Determines whether @a str is in the table.
     *
     * O(m) where m is the length of @a str
     */
    bool exists(const std::string& str) const
    {
        return exists(str.c_str());
    }

    /**
     * Gets the number of elements in the table.
     *
     * O(1)
     */
    size_t size() const
    {
        return _size;
    }

    /**
     * Determines whether the table is empty.
     *
     * O(1)
     */
    bool empty() const
    {
        return size() == 0;
    }

    /**
     * Gets the traits associated with this array hash.
     *
     * O(1)
     */
    const array_hash_traits &traits() const
    {
        return _traits;
    }

    /**
     * Inserts @a str into the table.
     *
     * O(m) where m is the length of @a str
     *
     * @param str  string to insert
     * @return  true if @a str is successfully inserted, false if @a str
     *          already appears in the table
     */
    bool insert(const char *str)
    {
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
            size_type current = *((size_type *) (p));
            size_type required = occupied + sizeof(length_type) + length;
            if (required > current) {
                _grow_slot(slot, current, required);
            }

            // Position for writing to the slot.
            p = _data[slot] + occupied - sizeof(length_type);

        } else {
            // Make a new slot for this string.
            size_type required = sizeof(size_type) + 2 * sizeof(length_type)
                    + length;
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
     * Inserts @a str into the table.
     *
     * O(m) where m is the length of @a str
     *
     * @param str  string to insert
     * @return  true iff @a str is successfully inserted, false if @a str
     *          already appears in the table
     */
    bool insert(const std::string& str)
    {
        return insert(str.c_str());
    }

    /**
     * Erases a string from the table.
     *
     * O(m) where m is the length of @a str
     *
     * @param str  string to erase
     * @return  instances of @a str that were erased
     */
    size_type erase(const char *str)
    {
        length_type length;
        int slot = _hash(str, length);
        char *p = _data[slot];
        if (p) {
            size_type occupied;
            if ((p = _search(str, p, length, occupied)) != NULL) {
                _erase_word(p, slot);
                return 1;
            }
        }
        return 0;
    }

    /**
     * Erases a string from the table.
     *
     * O(m) where m is the length of @a str
     *
     * @param str  string to erase
     * @return  instances of @a str that were erased
     */
    size_type erase(const std::string& str)
    {
        return erase(str.c_str());
    }

    /**
     * Erases a string from the hash table.
     *
     * O(1)
     *
     * @param pos  iterator to the string to erase
     */
    void erase(const iterator &pos)
    {
        // Only erase if the iterator does not point to the end
        // of the collection
        if (pos._p) {
            _erase_word(pos._p, pos._slot);
        }
    }

    /**
     * Clears all the elements from the hash table.
     *
     * O(n) where n is traits.slot_count
     */
    void clear()
    {
        _destroy();
        _init();
    }

    /**
     * Swaps information between two array hashes.
     *
     * O(1)
     */
    void swap(array_hash<std::string>& rhs)
    {
        std::swap(_data, rhs._data);
        std::swap(_size, rhs._size);
        std::swap(_traits, rhs._traits);
    }

    /**
     * Gets an iterator to the first element in the table.
     *
     * O(n) where n = traits.slot_count
     */
    iterator begin() const
    {
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
     *
     * O(1)
     */
    iterator end() const
    {
        return iterator(_traits.slot_count, NULL, _data, _traits.slot_count);
    }

    /**
     * Gets a reverse iterator to the first element in reverse order.
     *
     * O(1)
     */
    reverse_iterator rbegin() const
    {
        return reverse_iterator(end());
    }

    /**
     * Gets a reverse iterator to the last element in reverse order.
     *
     * O(n) where n = traits.slot_count
     */
    reverse_iterator rend() const
    {
        return reverse_iterator(begin());
    }

    /**
     * Searches for @a str in the table.
     *
     * O(m) where m is the length of @a str
     *
     * @param str  string to search for
     * @return  iterator to @a str in the table, or @a end() if @a str
     *          is not in the table
     */
    iterator find(const char *str) const
    {
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
        return iterator(slot, p, _data, _traits.slot_count);
    }

    /**
     * Searches for @a str in the table.
     *
     * O(m) where m is the length of @a str
     *
     * @param str  string to search for
     * @return  iterator to @a str in the table, or @a end() if @a str
     *          is not in the table
     */
    iterator find(const std::string& str) const
    {
        return find(str.c_str());
    }

    /**
     * Equality operator.
     *
     * O(n) where n = @a size()
     */
    bool operator==(const array_hash<std::string>& rhs)
    {
        if (size() == rhs.size()) {
            // don't want to do a memory comparison because traits
            // may differ
            iterator me = begin();
            iterator them = rhs.begin();
            iterator stop = end();
            while (me != stop) {
                if (strcmp(*me, *them) != 0) {
                    return false;
                }
                ++me;
                ++them;
            }
            return true;
        }
        return false;
    }

    /**
     * Inequality operator.
     *
     * O(n) where n = @a size
     */
    bool operator!=(const array_hash<std::string>& rhs)
    {
        return !operator==(rhs);
    }

    class iterator : public std::iterator<std::bidirectional_iterator_tag,
            const char *>
    {
        friend class array_hash;

    public:
        // correct the STL's assumption that this iterator is not a
        // const iterator
        typedef const char * reference;

        iterator() : _slot(0), _p(NULL), _data(NULL)
        {
        }

        /**
         * Move this iterator forward to the next element in the table.
         *
         * worst case O(n) where n = traits.slot_count
         *
         * Calling this function on an end() iterator does nothing.
         *
         * @return  self-reference
         */
        iterator& operator++()
        {
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
         * worst case O(n) where n = traits.slot_count
         *
         * Calling this function on a begin iterator does nothing.
         *
         * @return  self-reference
         */
        iterator& operator--()
        {
            if (_p) {
                // Find the iterator's current location in the slot
                char *next = _data[_slot] + sizeof(size_type);
                char *prev = next;
                while (next != _p) {
                    prev = next;
                    next += *((length_type *) next) + sizeof(length_type);
                }

                if (prev != next) {
                    // Move backwards in the current slot, then we're done
                    _p = prev;
                    return *this;
                } else {
                    // Move back to the previous occupied slot
                    int tmp = _slot;
                    --_slot;
                    while (_slot >= 0 && _data[_slot] == NULL) {
                        --_slot;
                    }

                    if (_slot < 0) {
                        // We are at the beginning. Make this a begin
                        // iterator
                        _slot = tmp;
                        return *this;
                    }
                }
            } else {
                // Subtracting from end(). Find the very last slot
                // in the table.
                _slot = _slot_count - 1;
                while (_data[_slot] == NULL) {
                    --_slot;
                }
            }

            // Move to the last element in this slot
            char *next = _data[_slot] + sizeof(size_type);
            while (*((length_type *)next) != 0) {
                _p = next;
                length_type l = *((length_type *)next);
                next += sizeof(length_type) + l;
            }
            return *this;
        }

        /**
         * Postfix increment operator.
         *
         * worst case O(n) where n = traits.slot_count
         */
        iterator operator++(int)
        {
            iterator result = *this;
            operator++();
            return result;
        }

        /**
         * Postfix decrement operator.
         *
         * worst case O(n) where n = traits.slot_count
         */
        iterator operator--(int)
        {
            iterator result = *this;
            operator--();
            return result;
        }

        /**
         * Iterator dereference operator.
         *
         * O(1)
         *
         * @return  character pointer to the string this iterator points to
         */
        const char *operator*() const
        {
            if (_p) {
                return _p + sizeof(length_type);
            }
            return NULL;
        }

        /**
         * Standard equality operator.
         *
         * O(1)
         */
        bool operator==(const iterator& rhs)
        {
            return _p == rhs._p;
        }

        /**
         * Standard inequality operator.
         *
         * O(1)
         */
        bool operator!=(const iterator& rhs)
        {
            return !operator==(rhs);
        }

    private:
        int _slot;
        char *_p;
        char **_data;
        int _slot_count;

        iterator(int slot, char *p, char **data, int slot_count) :
                _slot(slot), _p(p), _data(data), _slot_count(slot_count)
        {
        }
    };

private:
    array_hash_traits _traits;
    size_t _size;
    char **_data;

    /**
     * Initializes the internal data pointers.
     */
    void _init()
    {
        _data = new char *[_traits.slot_count];
        memset(_data, NULL, _traits.slot_count * sizeof(char*));
        _size = 0;
    }

    /**
     * Clears all the memory used by the table.
     */
    void _destroy()
    {
        for (int i = 0; i < _traits.slot_count; ++i) {
            delete[] _data[i];
        }
        delete[] _data;
        _data = NULL;
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
    int _hash(const char *str, length_type &length, int seed = 23) const
    {
        int h = seed;
        length = 0;
        while (str[length]) {
            // Hash this character.
            h = h ^ ((h << 5) + (h >> 2) + str[length]);
            ++length;
        }

        ++length; // include space for the NULL terminator
        return h & (_traits.slot_count - 1); // same as h %
                                             // _traits.slot_count if
                                             // _traits.slot_count is a
                                             // power of 2
    }

    /**
     * Searches for @a str in the table.
     *
     * @param str       string to search for
     * @param length    length of @a str
     * @param p         slot in @a data that @a str goes into
     * @param occupied  number of bytes in the slot that are currently in use.
     *                  This value is only meaningful when this function
     *                  doesn't find @a str
     *
     * @return  If @a str is found in the table, returns a pointer to
     *          the string and its corresponding length. If not, returns NULL.
     */
    char *_search(const char *str, char *p, length_type length,
            size_type &occupied) const
    {
        occupied = -1;
        char *start = p;

        // Search for str in the slot p points to.
        p += sizeof(size_type); // skip past size at beginning of slot
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
    void _grow_slot(int slot, size_type current, size_type required)
    {
        // Determine how much space the new slot needs.
        size_type new_size = current;
        if (_traits.allocation_chunk_size == 0) {
            new_size = required;
        } else {
            while (new_size < required) {
                new_size += _traits.allocation_chunk_size;
            }
        }

        // Make a new slot and copy all the data over.
        char *p = _data[slot];
        _data[slot] = new char[new_size];
        if (p != NULL) {
            memcpy(_data[slot], p, current);
            delete[] p;
        }
        *((size_type *) (_data[slot])) = new_size;
    }

    /**
     * Appends a string to a list of strings in a slot.
     *
     * Assumes the slot is big enough to hold the string.
     *
     * @param str     string to append
     * @param p       pointer to the location in the slot this string
     *                should occupy
     * @param length  length of @a str
     */
    void _append_string(const char *str, char *p, length_type length)
    {
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

    /**
     * Erases a word from a slot in the table.
     *
     * @param p     word to erase
     * @param slot  slot @a p is in
     */
    void _erase_word(char *p, int slot)
    {
        int length = *(length_type *) (p);
        size_type size = *((size_type *) _data[slot]);

        // Erase the word by overwriting it.
        int n = size - (p - _data[slot]);
        memcpy(p, p + sizeof(length_type) + length, n);

        // If that made the slot empty, erase the slot.
        if (*((length_type *) (_data[slot] + sizeof(size_type))) == 0) {
            delete[] _data[slot];
            _data[slot] = NULL;
        }
        --_size;
    }
};

} // namespace stx

#endif  // ARRAY_HASH_H
