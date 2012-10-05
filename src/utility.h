/** Utility class for Squeezegene, holding various useful, short, utility-like
functions.

Written I. Yulaev 2012-10-04
*/

#ifndef SQUEEZEGENE_UTILITY
#define SQUEEZEGENE_UTILITY

/** Compares two pairs having an int as the second member.
Returns true iff the int for pair one is less than the int in pair two. */
template<class T>
bool sort_pairs_by_second (pair<T, int> one, pair<T, int> two) {
	return(one.second < two.second);
}

#endif
