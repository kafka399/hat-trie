/**
 * @mainpage
 *
 * This project is intended to be a fully operational, standards compliant
 * HAT-trie.
 *
 * @section Implemented
 *
 * Here is a list of the major operations that are implemented:
 *
 * @li @c begin()
 * @li @c clear()
 * @li @c count(string)
 * @li @c empty()
 * @li @c end()
 * @li @c erase(string)
 * @li @c erase(iterator)
 * @li @c exists(string)
 * @li @c find(string)
 * @li @c insert(record)
 * @li @c insert(iterator, iterator)
 * @li @c size()
 * @li @c swap(hat_set &)
 * @li forward iteraton and iterator dereferencing
 *
 * In a @c hat_set, @c record is a @c std::string. In a @c hat_map, @c record
 * is a @c pair<std::string, T>.
 *
 * @section Future
 *
 * Here is a list of major operations that have yet to be implemented:
 *
 * @li reverse iteration
 * @li @c lower_bound(string)
 * @li @c upper_bound(string)
 * @li @c equal_range(string)
 * @li @c match_prefix(string) (an extension)
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
 * (See the Deviations section for differences.)
 *
 * @code
 * #include "hat_set.h"
 *
 * using namespace std;
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
 * @li @c match_prefix(string) -- returns a set of all strings that have
 * the parameter as a prefix. To be implemented.
 *
 * @section Deviations
 * The hat@_trie interface differs from the standard in a few ways:
 *
 * @li @c insert(record) -- returns a @c bool rather than a <tt> pair<iterator,
 * bool></tt>. See the HTML documentation for rationale.
 * @li iterator traversals are unordered. Ordered traversals are in the
 * works.
 *
 * @section Testing
 * The test files in the test/ directory achieve > 95% coverage of hat_trie.h
 * and array_hash.h.
 */

