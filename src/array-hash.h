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

      private:
        unsigned char *data;
        unsigned char *slot;
    };

  private:
    enum { SLOT_COUNT = 2048 };
    unsigned char **data;

    int hash(const char *str, uint16_t length, int seed = 23);
    void search(const char *str, uint16_t length);
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

void array_hash::search(const char *str, uint16_t length) {

}

void array_hash::insert(const char *str, uint16_t length) {
    // Find the length of @a str if necessary.
    if (length == 0) {
        length = strlen(str);
    }
    // Find the location to write to.
    int slot = hash(str, length);
    unsigned char *p = data[slot];
    if (data[slot]) {
        // Append the new string to the end of this slot.
        // How big does the new allocation need to be?
        uint32_t size = 0;
        uint16_t w = *((uint16_t *)data[slot]);
        while (w != 0) {
            data[slot] += sizeof(uint16_t);
            if (w == length) {
                // The string being scanned is the same length as @a str. Make
                // sure they aren't the same string.
                if (strncmp(str, (const char *)data[slot], length) == 0) {
                    data[slot] = p;
                    return;
                }
            }
            size += w + sizeof(uint16_t);
            data[slot] += w;
            w = *((uint16_t *)data[slot]);
        }
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
        } else {
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

array_hash::iterator::iterator() : data(NULL), slot(NULL) {

}


}  // namespace vaszauskas

#endif

