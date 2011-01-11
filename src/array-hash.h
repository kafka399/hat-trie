#ifndef ARRAY_HASH_H
#define ARRAY_HASH_H

#include <iostream>
#include <stdint.h>
#include <cstring>

using namespace std;

namespace vaszauskas {

class array_hash {
  public:
    array_hash();
    ~array_hash();

    void insert(const char *str, uint16_t length = 0);
    void print();

    class iterator {
      public:
        iterator();

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
    enum { SLOT_COUNT = 2048 };
    unsigned char **data;

    int hash(const char *str, uint16_t length, int seed = 23);
    uint32_t search(const char *str, uint16_t length, unsigned char *p = NULL);
};

array_hash::array_hash() {
    data = new unsigned char *[SLOT_COUNT];
    for (int i = 0; i < SLOT_COUNT; ++i) {
        data[i] = NULL;
    }
}

array_hash::~array_hash() {
    for (int i = 0; i < SLOT_COUNT; ++i) {
        delete data[i];
    }
    delete data;
}

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

int array_hash::hash(const char *str, uint16_t length, int seed) {
    int h = seed;
    for (uint16_t i = 0; i < length; ++i) {
        h = h ^ ((h << 5) + (h >> 2) + str[i]);
    }
    return h & (SLOT_COUNT - 1);
}

array_hash::iterator::iterator() : slot(0), p(NULL), data(NULL) {

}

array_hash::iterator& array_hash::iterator::operator++() {
    // Move data to the next string in this slot.
    data += *((uint16_t *)p) + sizeof(uint16_t);
    if (*((uint16_t *)p) == 0) {
        // Move down to the next slot.
        ++slot;
        //while (slot < SLOT_COUNT &&
    }
    return *this;
}

}  // namespace vaszauskas

#endif

