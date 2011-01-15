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
 */
class array_hash {
  private:
    typedef uint16_t length_type;
    typedef uint32_t size_type;

  public:
    class iterator;

    array_hash();
    ~array_hash();

    void insert(const char *str);
    bool find(const char *str) const;
    size_t size() const;

    iterator begin() const;
    iterator end() const;

    class iterator {
        friend class array_hash;

      public:
        iterator();
        iterator(const iterator& rhs);

        iterator& operator++();
        iterator& operator--();
        //pair<const char *, uint16_t> operator*();
        const char *operator*() const;
        bool operator==(const iterator& rhs);
        bool operator!=(const iterator& rhs);
        iterator& operator=(const iterator &rhs);

      private:
        int slot;
        char *p;
        char **data;
    };

  private:
    enum { SLOT_COUNT = 512 };  // MUST be a power of 2
    char **data;
    size_t _size;

    int hash(const char *str, length_type& length, int seed = 23) const;
    char *search(const char *str, length_type length, char *p) const;
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
    _size = 0;
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
 * @param p       slot in @a data that @a str goes into
 *
 * @return  If @a str is found in the table, returns a pointer to the string
 *          and its corresponding length. If not, returns NULL.
 */
char *array_hash::search(const char *str, length_type length, char *p) const {
    // Search for @a str in the slot pointed to by @a p.
    p += sizeof(size_type);  // skip past size at beginning of slot
    length_type w = *((length_type *)p);
    while (w != 0) {
        p += sizeof(length_type);
        if (w == length) {
            // The string being scanned is the same length as @a str. Make
            // sure they aren't the same string.
            if (strncmp(str, p, length) == 0) {
                // Found @a str.
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
 * @param str     string to insert
 */
void array_hash::insert(const char *str) {
  //cout << "INSERTING " << str << endl;
    length_type length;
    int slot = hash(str, length);
    char *p = data[slot];
    if (p) {
        // Append the new string to the end of this slot.
        if (search(str, length, p) != NULL) {
            // @a str is already in the table. Nothing needs to be done.
            return;
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
    // Write data for @a s.
    memcpy(p, &length, sizeof(length_type));
    p += sizeof(length_type);
    memcpy(p, str, length);
    p += length;
    length = 0;
    memcpy(p, &length, sizeof(length_type));
    ++_size;

    // debug print code
  //for (int i = 0; i < *((size_type *)(data[slot])); ++i) {
  //    cout << int(data[slot][i]) << " ";
  //}
  //cout << endl;
}

/**
 * Searches for @a str in the table.
 *
 * @param str     string to search for
 *
 * @return  true if @a str is in the table, false otherwise
 */
bool array_hash::find(const char *str) const {
    length_type length;
    char *p = data[hash(str, length)];
    if (p == NULL) {
        return false;
    }
    return search(str, length, p) != NULL;
}

/**
 * Gets the number of elements in the table.
 */
size_t array_hash::size() const {
    return _size;
}

/**
 * Gets an iterator to the first element in the table.
 */
array_hash::iterator array_hash::begin() const {
    iterator result;
    result.slot = 0;
    result.data = data;
    result.p = NULL;
    while (result.data[result.slot] == NULL) {
        ++result.slot;
    }
    result.p = result.data[result.slot] + sizeof(size_type);
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
 * @param length  length of @a str. This function calculates this value as it
 *                runs.
 * @param seed    seed for the hash function
 *
 * @return  hashed value of @a str, its slot in the table
 */
int array_hash::hash(const char *str, length_type& length, int seed) const {
    int h = seed;
    length = 0;
    while (str[length]) {
        h = h ^ ((h << 5) + (h >> 2) + str[length]);
        ++length;
    }
    ++length;  // include space for the NULL terminator
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
    p += *((length_type *)p) + sizeof(length_type);
    if (*((length_type *)p) == 0) {
        // Move down to the next slot.
        ++slot;
        while (data[slot] == NULL && slot < SLOT_COUNT) {
            ++slot;
        }
        if (slot == SLOT_COUNT) {
            p = NULL;
        } else {
            p = data[slot] + sizeof(size_type);
        }
    }
    return *this;
}

array_hash::iterator& array_hash::iterator::operator--() {
    // TODO
    return *this;
}

/**
 * Iterator dereference operator.
 *
 * @return  pair where @a first is a pointer to the (non-NULL terminated)
 *          string, and @a second is the length of the string
 */
/*
pair<const char *, uint16_t> array_hash::iterator::operator*() {
    pair<const char *, uint16_t> result;
    if (p) {
        result.first = p + sizeof(length_type);
        result.second = *((length_type *)(p));
    }
    return result;
}
*/
const char *array_hash::iterator::operator*() const {
    if (p) {
        return p + sizeof(length_type);
    }
    return NULL;
}

/**
 * Standard equality operator.
 */
bool array_hash::iterator::operator==(const iterator& rhs) {
    return p == rhs.p;
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

