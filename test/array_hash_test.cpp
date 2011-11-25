/*
 * array_hash_test.cpp
 *
 *  Created on: Nov 24, 2011
 *      Author: chris
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE arrayHash

#define TEST BOOST_AUTO_TEST_CASE

#include <string>
#include <set>
#include <stack>

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "../src/array_hash.h"

#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

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

BOOST_FIXTURE_TEST_SUITE(arrayHash, Data)

template <class A, class B>
void check_equal(const A& a, const B& b)
{
    set<string> x(a.begin(), a.end());
    set<string> y(b.begin(), b.end());
    BOOST_CHECK(x == y);
}

TEST(testConstructor)
{
    array_hash<string> ah;
    BOOST_CHECK(ah.find("") == ah.end());
    BOOST_CHECK(ah.begin() == ah.end());
    BOOST_CHECK(ah.size() == 0);
    BOOST_CHECK(ah.empty());
}

TEST(testExists)
{
    set<string> inserted;
    array_hash<string> ah;
    foreach (const string& str, data) {
        // keep track of what has been inserted already
        ah.insert(str);
        inserted.insert(str);

        // make sure the inserted data is the only data that appears
        // in the array hash
        foreach (const string& s, data) {
            BOOST_CHECK_EQUAL(inserted.find(s) != inserted.end(), ah.exists(s));
        }
    }
}

TEST(testFind)
{
    array_hash<string> ah(data.begin(), data.end());
    for (array_hash<string>::iterator it = ah.begin(); it != ah.end(); ++it) {
        // this performs a pointer comparison on the C-strings.
        // this is what we want because the iterators should
        // dereference to not only the same value, but the same
        // location in memory
        BOOST_CHECK(*(ah.find(*it)) == *it);
    }
}

TEST(testCopyConstructor)
{
    array_hash<string> a(data.begin(), data.end());
    array_hash<string>b(a);
    BOOST_CHECK(a == b);
}

TEST(testTraits)
{
    // Make an array hash with default values, then some more
    // array hashes with varying traits
    array_hash<string> a(data.begin(), data.end());
    array_hash_traits btraits(2, 0);
    array_hash<string> b(btraits);
    array_hash_traits ctraits(1048576, 700);
    array_hash<string> c(ctraits);

    // Copy the data to all the test array hashes
    foreach (const string& str, a) {
        b.insert(str);
        c.insert(str);
    }

    // Make sure all the hashes have the same values
    check_equal(a, b);
    check_equal(a, c);
}

TEST(testEraseByString)
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
    foreach (const string& str, data) {
        BOOST_CHECK_EQUAL(0, ah.erase(str));
    }
}

TEST(testEraseByIterator)
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
    foreach (const string& str, _data) {
        ah.erase(ah.find(str));
        check_equal(ah, data);
    }
}

TEST(testAssnOperator)
{
    array_hash<string> ah;
    ah.insert("hello");
    ah.insert("world");

    ah = array_hash<string>(data.begin(), data.end());
    check_equal(ah, data);
}

TEST(testInsert)
{
    array_hash<string> ah;
    foreach (const string& str, data) {
        BOOST_CHECK(ah.insert(str));
    }
    foreach (const string& str, data) {
        BOOST_CHECK(!ah.insert(str));
    }
}

TEST(testReverseIteration)
{
    // Initialize the stack
    array_hash<string> ah(data.begin(), data.end());
    stack<string> st;
    foreach (const string& str, ah) {
        st.push(str);
    }

    // Make sure the reverse iterator produces the same
    // values in reverse order
    for (array_hash<string>::reverse_iterator it = ah.rbegin(); it != ah.rend(); ++it) {
        BOOST_CHECK_EQUAL(st.top(), *it);
        st.pop();
    }
}

TEST(testIteratorBounds)
{
    array_hash<string> ah(data.begin(), data.end());
    array_hash<string>::const_iterator it = ah.begin();
    --it;
    BOOST_CHECK(it == ah.begin());
    it = ah.end();
    ++it;
    BOOST_CHECK(it == ah.end());
}

TEST(testEquality)
{
    array_hash<string> a(data.begin(), data.end());
    array_hash<string> b(data.begin(), data.end());
    BOOST_CHECK(a == b);

    array_hash<string> c;
    BOOST_CHECK(a != c);
}

TEST(testClear)
{
    array_hash<string> a(data.begin(), data.end());
    a.clear();
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.begin() == a.end());
    BOOST_CHECK(a.empty());
    BOOST_CHECK(a.find("") == a.end());
}

BOOST_AUTO_TEST_SUITE_END()
