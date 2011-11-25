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

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "../src/hat_set.h"

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
    }
};

BOOST_FIXTURE_TEST_SUITE(hatSet, Data)

TEST(testConstructor)
{
    hat_set<string> h;
    BOOST_CHECK(h.find("") == h.end());
    BOOST_CHECK(h.begin() == h.end());
    BOOST_CHECK(h.size() == 0);
    BOOST_CHECK(h.empty());
}

TEST(testExists)
{
    set<string> inserted;
    hat_set<string> ah;
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

BOOST_AUTO_TEST_SUITE_END()
