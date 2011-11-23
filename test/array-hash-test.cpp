/*
 * array-hash-test.cpp
 *
 *  Created on: Nov 22, 2011
 *      Author: chris
 */

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
    result.push_back(CUTE(testCopyConstructor));
    return result;
}

void arrayHashTest::testEmptyFind() {
    array_hash<string> ah;
    ASSERT(ah.find("") == ah.end());
    ASSERT(ah.find("hello") == ah.end());
}

void arrayHashTest::testExists() {
    // Make sure the data in the array hash and the
    // set are the same
}

void arrayHashTest::testFind() {

}

void arrayHashTest::testCopyConstructor() {
    array_hash<string> a;
    a.insert("a");
    a.insert("b");
    a.insert("c");
    a.insert("d");
    a.insert("e");
    array_hash<string> b(a);
    assert_equal(a, b);
}

set<string> arrayHashTest::testdata() {
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

array_hash<string> arrayHashTest::hashify(const set<string> &s) {
    // Create the array hash
    array_hash<string> ah;
    set<string> data = testdata();
    set<string>::iterator it;
    for (it = data.begin(); it != data.end(); ++it) {
        ah.insert(it->c_str());
    }
    return ah;
}

