/*
 * hat_set_test.cpp
 *
 *  Created on: Nov 24, 2011
 *      Author: chris
 */

#define BOOST_TEST_MODULE hatSet
#define TEST BOOST_AUTO_TEST_CASE

#include <string>
#include <set>
#include <stack>
#include <fstream>

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "../src/hat_set.h"

#define foreach BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

using namespace stx;
using namespace std;

struct HatTrieData
{
    set<string> data;

    HatTrieData()
    {
        ifstream file;
        file.open("inputs/kjv");
        if (!file) {
            throw "file not opened";
        }

        string reader;
        while (file >> reader) {
            data.insert(reader);
        }
    }
};

BOOST_FIXTURE_TEST_SUITE(hatSet, HatTrieData)

TEST(testConstructor)
{
    hat_set<string> h;
    BOOST_CHECK(h.begin() == h.end());
    BOOST_CHECK(h.size() == 0);
    BOOST_CHECK(h.empty());
}

TEST(testExists)
{
    hat_trie_traits traits;
    traits.burst_threshold = 2;
    hat_set<string> h(traits);
    h.insert("abcde");
    h.insert("abcd");
    h.insert("abc");
    h.insert("b");

    BOOST_CHECK(h.exists("a") == false);
    BOOST_CHECK(h.exists("abcde"));
    BOOST_CHECK(h.exists("ag") == false);
}

TEST(testFind)
{
    hat_trie_traits traits;
    traits.burst_threshold = 2;
    hat_set<string> h(traits);
    h.insert("abcde");
    h.insert("abcd");
    h.insert("abc");
    h.insert("b");
    BOOST_CHECK(h.find("a") == h.end());
    BOOST_CHECK_EQUAL(*h.find("b"), "b");
    BOOST_CHECK_EQUAL(*h.find("abcde"), "abcde");
    BOOST_CHECK(h.find("agf") == h.end());
    BOOST_CHECK(h.find("abcdefg") == h.end());
}

TEST(testInsert)
{
    hat_set<string> h;

    // Test c string insert
    BOOST_CHECK(h.insert("abc") == true);
    BOOST_CHECK(h.insert("ab") == true);
    BOOST_CHECK(h.insert("a") == true);
    BOOST_CHECK(h.insert("a") == false);

    // Test string insert
    string s = "abc";
    BOOST_CHECK(h.insert(s) == false);
    s = "abcd";
    BOOST_CHECK(h.insert(s) == true);

    // Test iterator insert
    hat_set<string>::iterator it = h.find("abc");
    BOOST_CHECK(*h.insert(it, "abcd") == "abcd");

    // Test range insert
    hat_set<string> a(data.begin(), data.end());
    BOOST_CHECK(a.size() == data.size());
}

BOOST_AUTO_TEST_SUITE_END()
