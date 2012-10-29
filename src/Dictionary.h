/** Dictionary is a class representing a loaded dictionary of strings, against which we will 
compress an uncompressed sequence file (consisting of letters 'A', 'C', 'G', 'T', case insensitive).

Written I. Yulaev 2012-10-15
*/

//#define DICTIONARY_DEBUG_LV1
#ifndef SQUEEZEGENE_DICTIONARY
#define SQUEEZEGENE_DICTIONARY

#include <vector>
#include "SequenceWord.h"

class Dictionary {
	private:	
	//Define the size of the dictionary we use, typically gets set to DICTIONARY_SIZE when this class is instantiated
	int dictionary_size;
	//dictionaryList will contain the loaded list of SequenceWords, that are loaded from the dictionary file
	//this is the data struture we use as our dictionary, it gets searched etc.
	std::vector<SequenceWord>* dictionaryList;
	
	/** Loads the given dictionaryfile into a vector of SequenceWords. The file had better already be sorted.
	Return: pointer to a newly-allocated vector of SequenceWords, that represents the loaded dictionary
	*/
	std::vector<SequenceWord>* loadDictionaryFile(const string & fileName);
	
	public:	
	/** Creates a new dictionary from dict file fileName, assumes that the dictionary has size n_dict_size */
	Dictionary(const string & fileName, int n_dict_size);
	~Dictionary();
	
	/** Return a copy of the sequenceword at position idx */
	SequenceWord* getWordAt(int idx) const;
	
	/** Returns the difference set between the provided SequenceWord and the dictionary SequenceWord at target_idx
	The return value is the difference list (expressed as a vector of bytes) where  if the byte starts with 1'b0, then the 
	next 15 bits represent the number of sequence symbols we should skip, and if the byte starts with 2'b10, then the
	next one byte represents a single symbol replacement and 2'b11 represents three symbol replacements.
	
	Parameters: query is the SequenceWord we compare against the dictionary, target_idx is the dictionary position
	that we are comparing query against*/
	std::vector<uint8_t>* calcStringDiffs(const SequenceWord & query, int target_idx) const;
	
	/** Try to find the index of an exact match between the provided SequenceWord and the 
	currently loaded dictionary. If a match is found, a dictionary index is returned, otherwise we 
	return (-1). */
	int findExactMatch(const SequenceWord & input) const;
	/** Try to find a match for input within the dictionary, changing at most one symbol in input. This gives (3*STR_LEN) possible
	matches to check against in the dictionary */
	int findNearMatch(const SequenceWord & input) const;
	/** Find the nearest match from the nearest DICTIONARY_NEAREST_SEARCH_RADIUS words, 
	centered around the dictionary's lower_bound() of the given SequenceWord input. */
	int findFromNearest(const SequenceWord & input) const;
};

#endif
