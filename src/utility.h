/** Utility class for Squeezegene, holding various useful, short, utility-like
functions.

Written I. Yulaev 2012-10-04
*/

#ifndef SQUEEZEGENE_UTILITY
#define SQUEEZEGENE_UTILITY

#include "sequenceword.h"

using namespace std;

/** Compares two pairs having an int as the second member.
Returns true iff the int for pair one is less than the int in pair two. */
template<class T>
bool sort_pairs_by_second (pair<T, int> one, pair<T, int> two) {
	return(one.second < two.second);
}

/** Creates hash for SequenceWord. Useful when creating an unordered_map of SequenceWords */
inline std::size_t sequenceWordHash(const SequenceWord & sw) {
	const uint64_t * rawdata = sw.getData();
	std::size_t retval = 0;
	
	for(int i = 0; i < ((STR_LEN/4) * sizeof(uint64_t) / sizeof(std::size_t)); i++) {
		//retval ^= ((std::size_t *) rawdata)[i];
	}
	
	return(retval);
}

/** Return the character (ASCII) corresponding to a code datum */
inline char codeToChar(uint8_t code) {
	switch(code & 0x3) {
		case 0x0: return('A');
		case 0x1: return('C');
		case 0x2: return('G');
		case 0x3: return('T');
	}
}

//functor for sequenceWordHash
typedef struct
{
	std::size_t operator() (SequenceWord k) const { return sequenceWordHash (k); }
} SequenceWordHash;


#endif
