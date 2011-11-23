// cute headers
#include "ide_listener.h"
#include "cute_runner.h"

// test headers
#include "array-hash-test.h"
#include "trie-test.h"

void runSuite() {
    cute::ide_listener lis;

    // Set up the array hash suite
    cute::makeRunner(lis)(arrayHashTest::suite(), "array hash");

    // Set up the hat trie suite
    cute::makeRunner(lis)(trieTest::suite(), "hat trie");
}

int main() {
    runSuite();
    return 0;
}



