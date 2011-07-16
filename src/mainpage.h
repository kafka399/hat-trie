/**
 * @mainpage
 *
 * This project is intended to be a fully operational, standards compliant
 * HAT-trie. It will eventually mirror the STL set interface, but for now,
 * it is still VERY much a work in progress.
 *
 * @section Implemented
 * Here is a list of the major operations that are implemented:
 *
 * @li @c begin()
 * @li @c clear()
 * @li @c count(string)
 * @li @c empty()
 * @li @c end()
 * @li @c exists(string)
 * @li @c find(string)
 * @li @c insert(record)
 * @li @c size()
 * @li @c swap(hat_set &)
 * @li forward iteraton and iterator dereferencing
 *
 * In a @c hat_set, @c record is a @c std::string. In a @c hat_map, @c record
 * is a @c pair<std::string, T>.
 *
 * @section Usage
 *
 * @subsection Installation
 * Copy all the headers into a directory in your PATH and include @c hat_set.h
 * in your project. Some of the headers require @c stdint.h, which isn't
 * available by default on most Windows platforms. You can find a compatible
 * version of the header on Google.
 *
 * All classes are defined in namespace stx.
 *
 * @subsection Example
 * In the vast majority of cases, these classes are used in exactly the
 * same way as their STL counterparts. Just replace @c set with @c hat_set.
 * @code
 * #include "hat_set.h"
 *
 * using namespace stx;
 *
 * int main() {
 *     hat_set<string> trie;
 *     trie.insert("hello world");
 *     cout << *trie.begin() << endl;
 *     return 0;
 * }
 * @endcode
 *
 * Prints <tt>hello world</tt>.
 *
 * @section Extensions
 * There are a few non-standard extensions to the hat_trie interface:
 *
 * @li @c exists(string) -- returns true iff there is a record in the trie
 * with a matching key
 *
 * @section Deviations
 * The hat@_trie interface differs from the standard in a few ways:
 *
 * @li @c insert(record) -- returns a @c bool rather than a <tt> pair<iterator,
 * bool></tt>. See the HTML documentation for rationale.
 */

