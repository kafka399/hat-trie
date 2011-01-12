#ifndef ARRAY_HASH_H
#define ARRAY_HASH_H

#include <iostream>
#include <stdint.h>
#include <cstring>
#include <utility>

using namespace std;

namespace stx {

class array_hash {
  public:
    class iterator;

    array_hash();
    ~array_hash();

    void insert(const char *str, uint16_t length = 0);
    void print();

    iterator begin();
    iterator end();

    class iterator {
        friend class array_hash;

      public:
        iterator();
        iterator(const iterator& rhs);

        iterator& operator++();
        iterator& operator--();
        pair<const char *, uint16_t> operator*();
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);
        iterator& operator=(const iterator &rhs);

      private:
        int slot;
        unsigned char *p;
        unsigned char **data;
    };

  private:
    enum { SLOT_COUNT = 2048 };  // MUST be a power of 2
    unsigned char **data;

    int hash(const char *str, uint16_t length, int seed = 23);
    uint32_t search(const char *str, uint16_t length, unsigned char *p = NULL);
};

/**
 * Standard default constructor.
 *
 * Creates a NULL-constructed table of slots for the array hash.
 */
array_hash::array_hash() {
    data = new unsigned char *[SLOT_COUNT];
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
 * Searches for @a str in the array hash.
 *
 * @param str     string to search for
 * @param length  length of @a str
 * @param p       slot in @a data that @a str goes into. If this value is
 *                NULL, the slot is calculated using @a hash().
 *
 * @return  If this function finds @a str in the array hash, returns 0. If
 *          not, returns the new size of the array that would need to be
 *          allocated to store the array and @a str.
 */
uint32_t array_hash::search(const char *str, uint16_t length, unsigned char *p) {
    if (p == NULL) {
        // Find the position in @a data for @a str.
        p = data[hash(str, length)];
    }
    // Search for @a str in this slot.
    uint32_t size = 0;
    uint16_t w = *((uint16_t *)p);
    while (w != 0) {
        p += sizeof(uint16_t);
        if (w == length) {
            // The string being scanned is the same length as @a str. Make
            // sure they aren't the same string.
            if (strncmp(str, (const char *)p, length) == 0) {
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
 * Inserts @a str into the tree.
 *
 * @param str     string to insert
 * @param length  length of @a str. If at all possible, this should be
 *                provided with by the caller. Calculating length separately
 *                slows this function down significantly.
 */
void array_hash::insert(const char *str, uint16_t length) {
    // Find the length of @a str if necessary.
    if (length == 0) {
        length = strlen(str);
    }
    // Find the location to write to.
    int slot = hash(str, length);
    unsigned char *p = data[slot];
    if (p) {
        // Append the new string to the end of this slot.
        uint32_t size = search(str, length, p);
        if (size == 0) {
            // @a str is already in the array hash. Return here.
            return;
        }
        // Append the new string to the end of this slot.
        data[slot] = new unsigned char[size + 2 * sizeof(uint16_t) + length];
        memcpy(data[slot], p, size);
        delete [] p;
        p = data[slot] + size;
    } else {
        // Make a new slot for this string.
        uint32_t size = length + 2 * sizeof(uint16_t);
        data[slot] = new unsigned char[size];
        p = data[slot];
    }
    // Write data for @a s.
    memcpy(p, &length, sizeof(uint16_t));
    p += sizeof(uint16_t);
    memcpy(p, str, length);
    p += length;
    length = 0;
    memcpy(p, &length, sizeof(uint16_t));
}

/**
 * Print out all the strings stored in the array hash.
 */
void array_hash::print() {
    for (int i = 0; i < SLOT_COUNT; ++i) {
        if (data[i]) {
            // Print out all strings in this array.
            unsigned char *p = data[i];
            uint16_t length = *((uint16_t *)p);
            while (length != 0) {
                p += sizeof(uint16_t);
                for (int i = 0; i < length; ++i) {
                    cout << p[i];
                }
                cout << endl;
                p += length;
                length = *((uint16_t *)p);
            }
        }
    }
}

/**
 * Gets an iterator to the first element in the hash table.
 */
array_hash::iterator array_hash::begin() {
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

array_hash::iterator array_hash::end() {
    iterator result;
    result.slot = SLOT_COUNT;
    result.data = data;
    result.p = NULL;
    return result;
}

/**
 * Hashing function for the array hash.
 *
 * @param str     string to hash
 * @param length  length of @a str
 * @param seed    seed for the hash function
 *
 * @return  Hashed value of @a str, the slot in @a data @a str should go into.
 */
int array_hash::hash(const char *str, uint16_t length, int seed) {
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
 * Dereference this iterator into its component parts.
 */
pair<const char *, uint16_t>
array_hash::iterator::operator*() {
    pair<const char *, uint16_t> result;
    if (p) {
        result.first = (const char *)(p) + 2;
        result.second = *((uint16_t *)p);
    } else {
        result.first = NULL;
        result.second = 0;
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

