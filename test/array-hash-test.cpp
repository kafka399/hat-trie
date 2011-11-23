/*
 * array-hash-test.cpp
 *
 *  Created on: Nov 22, 2011
 *      Author: chris
 */

#include "cute.h"
#include "array-hash-test.h"
#include "../src/array_hash.h"

#include <iostream>

using namespace std;
using namespace stx;

cute::suite arrayHashTest::suite() {
	cute::suite result;
	result.push_back(CUTE(testEmptyFind));
	return result;
}

void arrayHashTest::testEmptyFind() {
	array_hash<string> ah;
	ASSERT(ah.find("") == ah.end());
	ASSERT(ah.find("hello") == ah.end());
}

