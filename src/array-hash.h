#ifndef ARRAY_HASH_H
#define ARRAY_HASH_H

#include <iostream>
#include <stdint.h>
#include <cstring>
#include <utility>

using namespace std;

namespace stx {

/**
 * Hash table container for unsorted strings.
 *
 * All of the operations of this hash table can be SIGNIFICANTLY sped up if
 * the length of the string is provided as a parameter.
 */
class array_hash {
  public:
    class iterator;

    array_hash();
    ~array_hash();

    void insert(const char *str, uint16_t length = 0);
    bool find(const char *str, uint16_t length = 0) const;

    iterator begin() const;
    iterator end() const;

    class iterator {
        friend class array_hash;

      public:
        iterator();
        iterator(const iterator& rhs);

        iterator& operator++();
        iterator& operator--();
        const char *operator*();
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);
        iterator& operator=(const iterator &rhs);

      private:
        int slot;
        char *p;
        char **data;
    };

  private:
    enum { SLOT_COUNT = 2048 };  // MUST be a power of 2
    char **data;

    int hash(const char *str, uint16_t length, int seed = 23) const;
    uint32_t search(const char *str, uint16_t length, char *p = NULL) const;
};

// ----------
// array_hash
// ----------

/**
 * Standard default constructor.
 *
 * Creates a NULL-constructed table of slots.
 */
array_hash::array_hash() {
    data = new char *[SLOT_COUNT];
    for (int i = 0; i < SLOT_COUNT; ++i) {
        data[i] = NULL;
    }
}

/**
 * Standard destructor.
 */
array_hash::~array_hash() {
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
 * @param p       slot in @a data that @a str goes into. If this value is
 *                NULL, the slot is calculated using @a hash().
 *
 * @return  If this function finds @a str in the table, returns 0. If
 *          not, returns the new size of the array that would need to be
 *          allocated to store the array and @a str.
 */
uint32_t array_hash::search(const char *str, uint16_t length, char *p) const {
    if (p == NULL) {
        // Find the position in @a data for @a str.
        p = data[hash(str, length - 1)];  // don't hash the NULL terminator
    }
    // Search for @a str in this slot.
    uint32_t size = 0;
    uint16_t w = *((uint16_t *)p);
    while (w != 0) {
        p += sizeof(uint16_t);
        if (w == length) {
            // The string being scanned is the same length as @a str. Make
            // sure they aren't the same string.
            if (strcmp(str, p) == 0) {
                // Found @a str.
                return 0;
            }
        }
        size += w + sizeof(uint16_t);
        p += w;
        w = *((uint16_t *)p);
    }
    return size;
}

/**
 * Inserts @a str into the table.
 *
 * @param str     string to insert
 * @param length  length of @a str. If at all possible, this should be
 *                provided by the caller. Calculating length separately
 *                slows this function down significantly.
 */
void array_hash::insert(const char *str, uint16_t length) {
    // Find the length of @a str if necessary.
    if (length == 0) {
        length = strlen(str);
    }
    ++length;  // include space for the NULL terminator...
    int slot = hash(str, length - 1);   // ... but don't hash it
    char *p = data[slot];
    if (p) {
        // Append the new string to the end of this slot.
        uint32_t size = search(str, length, p);
        if (size == 0) {
            // @a str is already in the array hash. Return here.
            return;
        }
        // Append the new string to the end of this slot.
        data[slot] = new char[size + 2 * sizeof(uint16_t) + length];
        memcpy(data[slot], p, size);
        delete [] p;
        p = data[slot] + size;
    } else {
        // Make a new slot for this string.
        uint32_t size = length + 2 * sizeof(uint16_t);
        data[slot] = new char[size];
        p = data[slot];
    }
    // Write data for @a s.
    memcpy(p, &length, sizeof(uint16_t));
    p += sizeof(uint16_t);
    strcpy(p, str);
    p += length;
    length = 0;
    memcpy(p, &length, sizeof(uint16_t));
}

/**
 * Searches for @a str in the table.
 *
 * @param str     string to search for
 * @param length  length of @a str. If at all possible, this should be
 *                provided by the caller. Calculating length separately
 *                slows this function down significantly.
 *
 * @return  true if @a str is in the table, false otherwise
 */
bool array_hash::find(const char *str, uint16_t length) const {
    if (length == 0) {
        length = strlen(str) + 1;  // extra space for the NULL terminator
    }
    return search(str, length) == 0;
}

/**
 * Gets an iterator to the first element in the hash table.
 */
array_hash::iterator array_hash::begin() const {
    iterator result;
    result.slot = 0;
    result.data = data;
    result.p = NULL;
    while (result.data[result.slot] == NULL) {
        ++result.slot;
    }
    result.p = result.data[result.slot];
    return result;
}

/**
 * Gets an iterator to one past the last element in the hash table.
 */
array_hash::iterator array_hash::end() const {
    iterator result;
    result.slot = SLOT_COUNT;
    result.data = data;
    result.p = NULL;
    return result;
}

/**
 * Hashes @a str to an integer, its slot in the hash table.
 *
 * @param str     string to hash
 * @param length  length of @a str
 * @param seed    seed for the hash function
 *
 * @return  hashed value of @a str, its slot in the table
 */
int array_hash::hash(const char *str, uint16_t length, int seed) const {
    int h = seed;
    for (uint16_t i = 0; i < length; ++i) {
        h = h ^ ((h << 5) + (h >> 2) + str[i]);
    }
    return h & (SLOT_COUNT - 1);  // same as h % SLOT_COUNT if SLOT_COUNT
                                  // is a power of 2
}

// --------------------
// array_hash::iterator
// --------------------

/**
 * Standard default constructor.
 */
array_hash::iterator::iterator() : slot(0), p(NULL), data(NULL) {

}

/**
 * Standard copy constructor.
 */
array_hash::iterator::iterator(const iterator& rhs) {
    *this = rhs;
}

/**
 * Move this iterator forward to the next element in the array hash.
 *
 * @return  self-reference
 */
array_hash::iterator& array_hash::iterator::operator++() {
    // Move @a p to the next string in this slot.
    p += *((uint16_t *)p) + sizeof(uint16_t);
    if (*((uint16_t *)p) == 0) {
        // Move down to the next slot.
        ++slot;
        while (data[slot] == NULL && slot < SLOT_COUNT) {
            ++slot;
        }
        if (slot == SLOT_COUNT) {
            p = NULL;
        } else {
            p = data[slot];
        }
    }
    return *this;
}

/**
 * Iterator dereference operator.
 *
 * @return  pointer to the string represented by this iterator
 */
const char *array_hash::iterator::operator*() {
    const char *result = NULL;
    if (p) {
        result = p + sizeof(uint16_t);
    }
    return result;
}

/**
 * Standard equality operator.
 */
bool array_hash::iterator::operator==(const iterator& rhs) {
    return data == rhs.data && slot == rhs.slot && p == rhs.p;
}

/**
 * Standard inequality operator.
 */
bool array_hash::iterator::operator!=(const iterator& rhs) {
    return !(*this == rhs);
}

/**
 * Standard assignment operator.
 */
array_hash::iterator& array_hash::iterator::operator=(const iterator& rhs) {
    data = rhs.data;
    p = rhs.p;
    slot = rhs.slot;
    return *this;
}

}  // namespace stx

#endif

