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

/*
TEST(testExists)
{
    set<string> inserted;
    hat_set<string> hs;
    foreach (const string& str, data) {
        // keep track of what has been inserted already
        hs.insert(str);
        inserted.insert(str);

        // make sure the inserted data is the only data that appears
        // in the array hash
        foreach (const string& s, data) {
            BOOST_CHECK_EQUAL(inserted.find(s) != inserted.end(), hs.exists(s));
        }
    }
}
*/

TEST(testFind)
{
    hat_trie_traits traits;
    traits.burst_threshold = 2;
    hat_set<string> h(traits);
    h.insert("abcde");
    h.insert("abcd");
    h.insert("abc");
    h.insert("b");
    h.print();
    BOOST_CHECK(h.find("a") == h.end());
    BOOST_CHECK_EQUAL(*h.find("b"), "b");
    BOOST_CHECK_EQUAL(*h.find("abcde"), "abcde");
    BOOST_CHECK(h.find("agf") == h.end());
    BOOST_CHECK(h.find("abcdefg") == h.end());
}

BOOST_AUTO_TEST_SUITE_END()
