#ifndef ARRAY_HASH_TEST_H_
#define ARRAY_HASH_TEST_H_

// cute headers
#include "cute.h"

/**
 * Test class for array hash
 */
class arrayHashTest {
  public:
    static cute::suite suite();
  
  private:
    static void testEmptyFind();
};

#endif  // ARRAY_HASH_TEST_H_
