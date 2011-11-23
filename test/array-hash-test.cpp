// standard headers
#include <iostream>

// cute headers
#include "cute.h"

#include "array-hash-test.h"

using namespace std;
using namespace stx;

cute::suite arrayHashTest::suite() {
    cute::suite result;
    result.push_back(CUTE(testEmptyFind));
    result.push_back(CUTE(testExists));
    result.push_back(CUTE(testFind));
    result.push_back(CUTE(testCopyConstructor));
    result.push_back(CUTE(testTraits));
    result.push_back(CUTE(testErase));
    result.push_back(CUTE(testIteration));
    return result;
}

void arrayHashTest::testEmptyFind() {
    array_hash<string> ah;
    ASSERT(ah.find("") == ah.end());
    ASSERT(ah.find("hello") == ah.end());
    ASSERT(ah.begin() == ah.end());
}

void arrayHashTest::testExists() {
    // TODO
}

void arrayHashTest::testFind() {
    // TODO
}

void arrayHashTest::testCopyConstructor() {
    array_hash<string> a = hashify();
    array_hash<string> b(a);
    assert_equal(a, b);
}

void arrayHashTest::testTraits() {
    // Make an array hash with default values, then some more
    // array hashes with varying traits
    array_hash<string> a = hashify();
    array_hash_traits btraits(2, 0);
    array_hash<string> b(btraits);
    array_hash_traits ctraits(1048576, 700);
    array_hash<string> c(ctraits);

    // Copy the data to all the test array hashes
    array_hash<string>::iterator it;
    for (it = a.begin(); it != a.end(); ++it) {
        b.insert(*it);
        c.insert(*it);
    }

    // Make sure all the hashes have the same values
    assert_equal(a, b);
    assert_equal(a, c);
}

void arrayHashTest::testErase() {
    // TODO
}

void arrayHashTest::testIteration() {
    // TODO
}

//------------------------------------------------------------------------------
// private accessory functions
//------------------------------------------------------------------------------

set<string> arrayHashTest::getTestData() {
    static set<string> result;
    result.insert("hello");
    result.insert("chris");
    result.insert("");
    result.insert("abcd");
    return result;
}

template <class A, class B>
void arrayHashTest::assert_equal(const A& _a, const B& _b) {
    set<string> a = setify(_a);
    set<string> b = setify(_b);

    ASSERT_EQUAL(a.size(), b.size());
    set<string>::iterator ait = a.begin();
    set<string>::iterator bit = b.begin();
    for (; ait != a.end() && bit != b.end(); ++ait, ++bit) {
        ASSERT_EQUAL(*ait, *bit);
    }
    ASSERT(ait == a.end());
    ASSERT(bit == b.end());
}

template <class A>
set<string> arrayHashTest::setify(const A& a) {
    return set<string>(a.begin(), a.end());
}

array_hash<string> arrayHashTest::hashify(const set<string> &data) {
    // Create the array hash
    array_hash<string> ah;
    set<string>::iterator it;
    for (it = data.begin(); it != data.end(); ++it) {
        ah.insert(it->c_str());
    }
    return ah;
}

