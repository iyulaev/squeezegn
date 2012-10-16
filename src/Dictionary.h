/** Dictionary is a ...
*/

//#define DICTIONARY_DEBUG_LV1
#ifndef SQUEEZEGENE_DICTIONARY
#define SQUEEZEGENE_DICTIONARY

#include <vector>
#include "SequenceWord.h"

class Dictionary {
	public:	
	int dictionary_size;
	std::vector<SequenceWord>* dictionaryList;
	std::vector<SequenceWord>* loadDictionaryFile(string fileName);
	
	public:
	Dictionary(string fileName, int n_dict_size);
	
	//Target represents the INDEX of the target (dictionary) string which we compare against
	std::vector<uint8_t>* calcStringDiffs(SequenceWord query, int target_idx);
	
	int findExactMatch(SequenceWord input);
	int findNearMatch(SequenceWord input);
	int findFromNearest(SequenceWord input);
};

#endif
