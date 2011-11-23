#ifndef ARRAY_HASH_TEST_H_
#define ARRAY_HASH_TEST_H_

// standard headers
#include <set>
#include <string>

// cute headers
#include "cute.h"

// array hash headers
#include "../src/array_hash.h"

/**
 * Test class for array hash
 */
class arrayHashTest {
  public:
    static cute::suite suite();

  private:
    static void testEmptyFind();
    static void testExists();
    static void testFind();
    static void testCopyConstructor();
    static void testTraits();
    static void testErase();
    static void testIteration();

    static std::set<std::string> getTestData();

    template <class A, class B>
    static void assert_equal(const A& a, const B& b);
    template <class A>
    static std::set<std::string> setify(const A& a);
    static stx::array_hash<std::string> hashify(const std::set<std::string> &s = getTestData());
};

#endif  // ARRAY_HASH_TEST_H_
