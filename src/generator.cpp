#include <cstdlib>  // for rand
#include <iostream>

using namespace std;

int main() {
    srand(time(NULL));

    // generate a ton of variable length random strings
    for (int i = 0; i < 5000000; ++i) {
        for (int j = 0; j < rand() % 15 + 5; ++j) {
            //cout << char(rand() % 74 + 48); // 0-9:;<=>?@A-Z[\]^_`a-z
            cout << char(rand() % 26 + 97); // a-z
        }
        cout << endl;
    }
}

