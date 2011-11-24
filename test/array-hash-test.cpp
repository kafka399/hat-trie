// standard headers
#include <iostream>
#include <stack>

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
    result.push_back(CUTE(testEraseString));
    result.push_back(CUTE(testAssnOperator));
    result.push_back(CUTE(testInsert));
    result.push_back(CUTE(testReverseIteration));
    return result;
}

void arrayHashTest::testEmptyFind() {
    array_hash<string> ah;
    ASSERT(ah.find("") == ah.end());
    ASSERT(ah.find("hello world") == ah.end());
    ASSERT(ah.begin() == ah.end());
}

void arrayHashTest::testExists() {
    set<string> data = getTestData();
    set<string> inserted;
    array_hash<string> ah;
    for (set<string>::iterator sit = data.begin(); sit != data.end(); ++sit) {
        // insert the data into the array hash
        ah.insert(sit->c_str());
        inserted.insert(*sit);

        // make sure the inserted data is the only data that appears
        // in the array hash
        set<string>::iterator it;
        for (it = data.begin(); it != data.end(); ++it) {
            ASSERT_EQUAL(inserted.find(*it) != inserted.end(),
                    ah.exists(it->c_str()));
        }
    }
}

void arrayHashTest::testFind() {
    array_hash<string> ah = hashify();
    array_hash<string>::iterator it;
    for (it = ah.begin(); it != ah.end(); ++it) {
        // this performs a pointer comparison on the C-strings.
        // this is what we want because the iterators should
        // dereference to not only the same value, but the same
        // location in memory
        ASSERT(*(ah.find(*it)) == *it);
    }
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

void arrayHashTest::testEraseString() {
    array_hash<string> ah = hashify();
    set<string> data = getTestData();

    while (data.empty() == false) {
        ASSERT_EQUAL(1, ah.erase(data.begin()->c_str()));
        data.erase(data.begin());
        assert_equal(ah, data);
    }

    // Make sure the return value for erase is OK
    data = getTestData();
    for (set<string>::iterator it = data.begin(); it != data.end(); ++it) {
        ASSERT_EQUAL(0, ah.erase(it->c_str()));
    }
}

void arrayHashTest::testAssnOperator() {
    array_hash<string> ah;
    ah.insert("hello");
    ah.insert("world");

    ah = hashify();
    set<string> set = getTestData();
    assert_equal(ah, set);
}

void arrayHashTest::testInsert() {
    array_hash<string> ah;
    set<string> data = getTestData();
    for (set<string>::iterator it = data.begin(); it != data.end(); ++it) {
        ASSERT(ah.insert(it->c_str()));
    }
    for (set<string>::iterator it = data.begin(); it != data.end(); ++it) {
        ASSERT(!ah.insert(it->c_str()));
    }
}

void arrayHashTest::testReverseIteration() {
    stack<string> st;

    // Initialize the stack
    array_hash<string> ah = hashify();
    for (array_hash<string>::iterator it = ah.begin(); it != ah.end(); ++it) {
        st.push(*it);
    }

    // Make sure the reverse iterator produces the same
    // values in reverse order
    for (array_hash<string>::reverse_iterator it = ah.rbegin(); it != ah.rend(); ++it) {
        ASSERT_EQUAL(st.top(), *it);
        st.pop();
    }

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

