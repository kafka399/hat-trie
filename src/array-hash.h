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

  public:
    enum { SLOT_COUNT = 512 };
    unsigned char **data;

    int hash(const char *str, uint16_t length, int seed = 23);
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

void array_hash::insert(const char *str, uint16_t length) {
    int slot = hash(str, length);
    //cout << "INSERTING " << str << " INTO SLOT " << slot << endl;
    if (length == 0) {
        length = strlen(str);
    }
    if (data[slot]) {
        // Append the new string to the end of this slot.
        uint32_t size = *((uint32_t *)data[slot]) + length + sizeof(uint16_t);
        unsigned char *p = data[slot];
        data[slot] = new unsigned char[size];
        memcpy(data[slot], p, *((uint32_t *)p));
        delete p;
        p = data[slot] + size - length - sizeof(uint16_t);
        memcpy(data[slot], &size, sizeof(uint32_t));
        memcpy(p - sizeof(uint16_t), &length, sizeof(uint16_t));
        memcpy(p, str, length);
        p += length;
        length = 0;
        memcpy(p, &length, sizeof(uint16_t));

      //for (int i = 0; i < size; ++i) {
      //    int8_t ch = data[slot][i];
      //    cout << int(ch) << " ";
      //}
      //cout << endl;
    } else {
        // Make a new slot for this string.
        uint32_t size = sizeof(uint32_t) + length + 2 * sizeof(uint16_t);
        data[slot] = new unsigned char[size];
        unsigned char *p = data[slot];
        memcpy(p, &size, sizeof(uint32_t));
        p += sizeof(uint32_t);
        memcpy(p, &length, sizeof(uint16_t));
        p += sizeof(uint16_t);
        memcpy(p, str, length);
        p += length;
        length = 0;
        memcpy(p, &length, sizeof(uint16_t));

      //for (int i = 0; i < size; ++i) {
      //    int8_t ch = data[slot][i];
      //    cout << int(ch) << " ";
      //}
      //cout << endl;
    }
}

void array_hash::print() {
    for (int i = 0; i < SLOT_COUNT; ++i) {
        if (data[i]) {
            // Print out all strings in this array.
            unsigned char *p = data[i];
            p += sizeof(uint32_t);  // skip allocated size
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

}  // namespace vaszauskas

#endif

