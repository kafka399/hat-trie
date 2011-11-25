/*
 * array_hash_test.cpp
 *
 *  Created on: Nov 24, 2011
 *      Author: chris
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE array_hash

#define CASE BOOST_AUTO_TEST_CASE

#include <boost/test/unit_test.hpp>
#include <string>
#include <set>
#include <stack>

#include "../src/array_hash.h"

using namespace stx;
using namespace std;

struct Data
{
    set<string> data;

    Data()
    {
        data.insert("");
        data.insert("a");
        data.insert("ab");
        data.insert("abc");
    }
};

BOOST_FIXTURE_TEST_SUITE(ArrayHash, Data)

template <class A, class B>
void check_equal(const A& a, const B& b)
{
    set<string> x(a.begin(), a.end());
    set<string> y(b.begin(), b.end());
    BOOST_REQUIRE(x == y);
}

CASE(testConstructor)
{
    array_hash<string> ah;
    BOOST_CHECK(ah.find("") == ah.end());
    BOOST_CHECK(ah.begin() == ah.end());
    BOOST_CHECK(ah.size() == 0);
}

CASE(testExists)
{
    set<string> inserted;
    array_hash<string> ah;
    for (set<string>::iterator sit = data.begin(); sit != data.end(); ++sit) {
        // keep track of what has been inserted already
        ah.insert(*sit);
        inserted.insert(*sit);

        // make sure the inserted data is the only data that appears
        // in the array hash
        set<string>::iterator it;
        for (it = data.begin(); it != data.end(); ++it) {
            BOOST_REQUIRE_EQUAL(
                    inserted.find(*it) != inserted.end(), ah.exists(*it));
        }
    }
}

CASE(testFind)
{
    array_hash<string> ah(data.begin(), data.end());
    for (array_hash<string>::iterator it = ah.begin(); it != ah.end(); ++it) {
        // this performs a pointer comparison on the C-strings.
        // this is what we want because the iterators should
        // dereference to not only the same value, but the same
        // location in memory
        BOOST_REQUIRE(*(ah.find(*it)) == *it);
    }
}

CASE(testCopyConstructor)
{
    array_hash<string> a(data.begin(), data.end());
    array_hash<string>b(a);
    BOOST_REQUIRE(a == b);
}

CASE(testTraits)
{
    // Make an array hash with default values, then some more
    // array hashes with varying traits
    array_hash<string> a(data.begin(), data.end());
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
    check_equal(a, b);
    check_equal(a, c);
}

CASE(testEraseByString)
{
    array_hash<string> ah(data.begin(), data.end());
    set<string> _data(data);

    // Erase values from the hash
    while (data.empty() == false) {
        BOOST_CHECK_EQUAL(1, ah.erase(*data.begin()));
        data.erase(data.begin());
        check_equal(ah, data);
    }

    // Make sure empty erase works too
    data = _data;
    for (set<string>::iterator it = data.begin(); it != data.end(); ++it) {
        BOOST_CHECK_EQUAL(0, ah.erase(*it));
    }
}

CASE(testEraseByIterator)
{
    array_hash<string> ah(data.begin(), data.end());
    set<string> _data(data);

    // Erase values from the hash
    while (data.empty() == false) {
        ah.erase(ah.find(*data.begin()));
        data.erase(data.begin());
        check_equal(ah, data);
    }

    // Make sure empty erase works too
    for (set<string>::iterator it = _data.begin(); it != _data.end(); ++it) {
        ah.erase(ah.find(*it));
        check_equal(ah, data);
    }
}

CASE(testAssnOperator)
{
    array_hash<string> ah;
    ah.insert("hello");
    ah.insert("world");

    ah = array_hash<string>(data.begin(), data.end());
    check_equal(ah, data);
}

CASE(testInsert)
{
    array_hash<string> ah;
    for (set<string>::iterator it = data.begin(); it != data.end(); ++it) {
        BOOST_CHECK(ah.insert(*it));
    }
    for (set<string>::iterator it = data.begin(); it != data.end(); ++it) {
        BOOST_CHECK(!ah.insert(*it));
    }
}

CASE(testReverseIteration)
{
    // Initialize the stack
    array_hash<string> ah(data.begin(), data.end());
    stack<string> st;
    for (array_hash<string>::iterator it = ah.begin(); it != ah.end(); ++it) {
        st.push(*it);
    }

    // Make sure the reverse iterator produces the same
    // values in reverse order
    for (array_hash<string>::reverse_iterator it = ah.rbegin(); it != ah.rend(); ++it) {
        BOOST_CHECK_EQUAL(st.top(), *it);
        st.pop();
    }
}

CASE(testIteratorBounds)
{
    array_hash<string> ah(data.begin(), data.end());
    array_hash<string>::const_iterator it = ah.begin();
    --it;
    BOOST_CHECK(it == ah.begin());
    it = ah.end();
    ++it;
    BOOST_CHECK(it == ah.end());
}

CASE(testEquality)
{
    array_hash<string> a(data.begin(), data.end());
    array_hash<string> b(data.begin(), data.end());
    BOOST_CHECK(a == b);

    array_hash<string> c;
    BOOST_CHECK(a != c);
}

BOOST_AUTO_TEST_SUITE_END()
