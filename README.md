# HAT-trie
This project is intended to be a fully operational, standards compliant
HAT-trie. It will eventually mirror the STL set interface, but for now,
it is still _VERY_ much a work in progress.

## Implemented
Here is a list of the major operations that are implemented:
 
 * ``begin()``
 * ``clear()``
 * ``count(string)``
 * ``empty()``
 * ``end()``
 * ``exists(string)``
 * ``find(string)``
 * ``insert(record)``
 * ``size()``
 * ``swap(hat_set &)``
 * forward iteraton and iterator dereferencing

In a ``hat_set``, ``record`` is a ``std::string``. In a ``hat_map``, ``record``
is a ``pair<std::string, T>``.

## Usage

### Installation
Copy all the headers into a directory in your PATH and include ``hat_set.h`` in
your project. Some of the headers require ``stdint.h``, which isn't available
by default on most Windows platforms. You can find a compatible version of the
header on Google.

All classes are defined in namespace stx.

### Example
In the vast majority of cases, these classes are used in exactly the
same way as their STL counterparts. Just replace ``set`` with ``hat_set``.

    #include "hat_set.h"

    using namespace stx;

    int main() {
        hat_set<string> trie;
        trie.insert("hello world");
        cout << *trie.begin() << endl;
        return 0;
    }

Prints ``hello world``.

## Extensions
There are a few non-standard extensions to the hat\_trie interface:

* ``exists(string)`` -- returns true iff there is a record in the trie with a
  matching key

## Deviations
The hat\_trie interface differs from the standard in a few ways:

* ``insert(record)`` -- returns a ``bool`` rather than a ``pair<iterator,
  bool>``. See the HTML documentation for rationale.
