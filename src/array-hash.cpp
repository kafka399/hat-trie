#include "array-hash.h"

namespace stx {

// ----------
// ht_array_hash
// ----------

/**
 * Default constructor.
 */
ht_array_hash::
ht_array_hash() {
    data = new char *[SLOT_COUNT];
    for (int i = 0; i < SLOT_COUNT; ++i) {
        data[i] = NULL;
    }
    _size = 0;
    ALLOCATION_CHUNK_SIZE = 32;
}

/**
 * Standard destructor.
 */
ht_array_hash::
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
char *ht_array_hash::
_search(const char *str, length_type length, char *p) const {
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
 * @return  true if @a str is successfully inserted, false if @a str already
 *          appears in the table
 */
bool ht_array_hash::
insert(const char *str) {
    length_type length;
    int slot = _hash(str, length);
    char *p = data[slot];
    if (p) {
        // Append the new string to the end of this slot.
        if (_search(str, length, p) != NULL) {
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
        //data[slot] = new char[ALLOCATION_CHUNK_SIZE];
        //*((size_type *)(data[slot])) = ALLOCATION_CHUNK_SIZE;
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

    // We wrote str into the table, so return true.
    return true;
}

/**
 * Searches for @a str in the table.
 *
 * @param str  string to search for
 * @return  true iff @a str is in the table
 */
bool ht_array_hash::
contains(const char *str) const {
    // Determine which slot in the table should contain str.
    length_type length;
    char *p = data[_hash(str, length)];

    // Return true if p is in that slot.
    if (p == NULL) {
        return false;
    }
    return _search(str, length, p) != NULL;
}

/**
 * Searches for @a str in the table.
 *
 * @param str  string to search for
 * @return  iterator to @a str in the table, or @a end() if @a str
 *          is not in the table
 */
ht_array_hash::iterator
ht_array_hash::
find(const char *str) const {
    // Determine which slot in the table should contain str.
    length_type length;
    int slot = _hash(str, length);
    char *p = data[slot];

    // Search for str in that slot.
    if (p == NULL) {
        return end();
    }
    p = _search(str, length, p);
    return iterator(slot, p, data);
}

/**
 * Gets the number of elements in the table.
 */
size_t ht_array_hash::
size() const {
    return _size;
}

/**
 * Gets an iterator to the first element in the table.
 */
ht_array_hash::iterator
ht_array_hash::
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
ht_array_hash::iterator
ht_array_hash::
end() const {
    return iterator(SLOT_COUNT, NULL, data);
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
int ht_array_hash::
_hash(const char *str, length_type& length, int seed) const {
    int h = seed;
    length = 0;
    while (str[length]) {
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
ht_array_hash::
iterator::iterator() :
        slot(0), p(NULL), data(NULL) {

}

/**
 * Standard copy constructor.
 */
ht_array_hash::
iterator::iterator(const iterator& rhs) {
    *this = rhs;
}

/**
 * Move this iterator forward to the next element in the array hash.
 *
 * @return  self-reference
 */
ht_array_hash::iterator&
ht_array_hash::
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
                // We are at the end. Make this an end iterator
                p = NULL;
            } else {
                // Move to the first element in this slot.
                p = data[slot] + sizeof(size_type);
            }
        }
    }
    return *this;
}

ht_array_hash::iterator&
ht_array_hash::
iterator::operator--() {
    // TODO
    return *this;
}

/**
 * Iterator dereference operator.
 *
 * @return  character pointer to the string this iterator points to
 */
const char *ht_array_hash::
iterator::operator*() const {
    if (p) {
        return p + sizeof(length_type);
    }
    return NULL;
}

/**
 * Standard equality operator.
 */
bool ht_array_hash::
iterator::operator==(const iterator& rhs) {
    return p == rhs.p;
}

/**
 * Standard inequality operator.
 */
bool ht_array_hash::
iterator::operator!=(const iterator& rhs) {
    return !operator==(rhs);
}

}  // namespace stx

