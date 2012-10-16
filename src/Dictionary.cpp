#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <exception>

#include "squeezegene.h"
#include "utility.h"

#include "Dictionary.h"

using namespace std;

/** Creates a new dictionary from dict file fileName, assumes that the dictionary has size n_dict_size
*/
Dictionary::Dictionary(string fileName, int n_dict_size) {
	dictionary_size = n_dict_size;
	dictionaryList = loadDictionaryFile(fileName);
}

/** Loads the given dictionaryfile into a vector of SequenceWords

Return: pointer to a newly-allocated vector of SequenceWords, that represents the loaded dictionary
*/
vector<SequenceWord>* Dictionary::loadDictionaryFile(string fileName) {
	ifstream myfile;
	vector<SequenceWord>* retval = new vector<SequenceWord>();
	
	myfile.open(fileName.c_str());
	
	if(!myfile.good()) {
		cerr << "Dictionary file could not be opened!" << endl;
		throw EXCEPTION_FILE_IO;
	}
	
	int i = 0;
	while(myfile.good()) {
		char filebuf[STR_LEN+1];
		myfile.getline(filebuf, STR_LEN+1);
		retval->push_back(SequenceWord(filebuf));
		
		#ifdef DICTIONARY_DEBUG_LV1
		char temp[STR_LEN+1];
		retval->back().outputStr(temp);
		printf("Pushed string to dict: %s\n", temp);
		
		if(!myfile.good()) {
			printf("myfile stopped being good after line %d\n", i);
		}
		#endif
		i++;
	}
	myfile.close();	
		
	return(retval);
}

/** Returns the difference set between the provided SequenceWord and the dictionary SequenceWord at target_idx
The return value is the difference list (expressed as a vector of bytes) where  if the byte starts with 1'b0, then the 
next 15 bits represent the number of sequence symbols we should skip, and if the byte starts with 2'b10, then the
next one byte represents a single symbol replacement and 2'b11 represents three symbol replacements.
*/
vector<uint8_t>* Dictionary::calcStringDiffs(SequenceWord query, int target_idx) {
	SequenceWord target = (*dictionaryList)[target_idx];
	
	#ifdef DICTIONARY_DEBUG_LV1
	printf("calcStringDiffs() comparing the following strings\n");
	char temp[STR_LEN+1];
	query.outputStr(temp);
	printf("Q: %s\n", temp);
	target.outputStr(temp);
	printf("T: %s\n", temp);
	#endif
	
	if(target == query) { return(NULL); }
	
	vector<uint8_t>* retval = new vector<uint8_t>();
	uint16_t skipped_letters = 0;;
	
	int i = 0;
	while(i < STR_LEN) {
		if(query.getDatumAt(i) != target.getDatumAt(i)) {
			#ifdef DICTIONARY_DEBUG_LV1
			fprintf(stderr, "Determined that datums 0x%02x and 0x%02x were unequal!\n",
				query.getDatumAt(i),
				target.getDatumAt(i));
			#endif
		
			if(skipped_letters > 0) {
				uint16_t skipword = skipped_letters & 0x7F;
				retval->push_back((uint8_t)((skipword >> 8) & 0xFF));
				retval->push_back((uint8_t) (skipword & 0xFF));
				skipped_letters = 0;
			}
		
			if(i > (STR_LEN - 3)) {
				uint8_t diff = 0x80 | (query.getDatumAt(i));
				retval->push_back(diff);
				i++;
			} else {
				uint8_t diff = 0xC0 | ((query.getDatumAt(i)<<4)|(query.getDatumAt(i+1)<<2)|(query.getDatumAt(i+2)));
				retval->push_back(diff);
				i+=3;
			}
		}
		
		else {
			i++;
			skipped_letters++;
		}
	}
	
	if(skipped_letters > 0) {
		uint16_t skipword = skipped_letters & 0x7F;
		retval->push_back((uint8_t)((skipword >> 8) & 0xFF));
		retval->push_back((uint8_t) (skipword & 0xFF));
		skipped_letters = 0;
	}
	
	return(retval);
}

//#define DEBUG_DICTIONARY_FEM
/** Try to find the index of an exact match between the provided SequenceWord and the 
currently loaded dictionary. If a match is found, a dictionary index is returned, otherwise we 
return (-1). */
int Dictionary::findExactMatch(SequenceWord input) {
	auto it_first = dictionaryList->begin();
	auto it_last = dictionaryList->end();

	//Performs binary search across dictionary for word equal or greater than input
	auto it_hit = lower_bound(it_first, it_last, input);
	
	#ifdef DEBUG_DICTIONARY_FEM
	printf("idx = %d\n", (it_hit - it_first));
	#endif

	SequenceWord lowerBoundWord = *it_hit;
		
	if(lowerBoundWord.isEqual(input)) {
		return (it_hit - it_first);
	}
	
	else {
		return(-1);
	}
}

/** Find the nearest match from the nearest DICTIONARY_NEAREST_SEARCH_RADIUS words, 
centered around the dictionary's lower_bound() of the given SequenceWord input. */
int Dictionary::findFromNearest(SequenceWord input) {
	auto it_first = dictionaryList->begin();
	auto it_last = dictionaryList->end();

	//Performs binary search across dictionary for word equal or greater than input
	auto it_hit = lower_bound(it_first, it_last, input);
	
	int min_diff = STR_LEN+1; 
	int min_pos = -1;
	
	auto it_hit_less = it_hit;
	while(it_hit_less > it_first && (it_hit-it_hit_less)<DICTIONARY_NEAREST_SEARCH_RADIUS) {
		it_hit_less--;
		if(input.calcDiff(*it_hit_less) < min_diff) {
			min_diff = input.calcDiff(*it_hit_less);
			min_pos = it_first - it_hit_less;
		}
	}
	
	auto it_hit_greater = it_hit;
	while(it_hit_greater < it_last && (it_hit_greater-it_hit)<DICTIONARY_NEAREST_SEARCH_RADIUS) {
		it_hit_greater++;
		if(input.calcDiff(*it_hit_greater) < min_diff) {
			min_diff = input.calcDiff(*it_hit_greater);
			min_pos = it_first - it_hit_greater;
		}
	}
	
	return(min_pos);
}

/** Try to find a match for input within the dictionary, changing at most one symbol in input. This gives (3*STR_LEN) possible
matches to check against in the dictionary */
int Dictionary::findNearMatch(SequenceWord input) {
	//change each character in input by 1
	for(int i = 0; i < STR_LEN; i++) {
		//there are thre epossible options for each character
		for(int j = 0; j < 3; j++) {
			SequenceWord trial(input);
			trial.pushDatum(trial.getDatumAt(i) + j % 4, i);
			
			int position = findExactMatch(trial);
			
			if(position != -1) { return(position); }
		}
	}
	
	return(-1);
}







