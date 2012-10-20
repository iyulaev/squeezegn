#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <fstream>
#include <exception>

#include "squeezegene.h"
#include "utility.h"

#include "Dictionary.h"

using namespace std;

Dictionary::Dictionary(string fileName, int n_dict_size) {
	dictionary_size = n_dict_size;
	dictionaryList = loadDictionaryFile(fileName);
}

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
		
		if(strlen(filebuf) >= STR_LEN) {
			retval->push_back(SequenceWord(filebuf));
			
			#ifdef DICTIONARY_DEBUG_LV1
			char temp[STR_LEN+1];
			retval->back().outputStr(temp);
			printf("Pushed string to dict: %s\n", temp);
			#endif
		}
		#ifdef DICTIONARY_DEBUG_LV1
		if(!myfile.good()) {
			printf("myfile stopped being good after line %d\n", i);
		}
		#endif
		i++;
	}
	myfile.close();	
		
	return(retval);
}


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
		
			//Skip letter (16 bit sequence) starts with 1'b0
			if(skipped_letters > 0) {
				uint16_t skipword = skipped_letters & 0x7F;
				retval->push_back((uint8_t)((skipword >> 8) & 0xFF));
				retval->push_back((uint8_t) (skipword & 0xFF));
				skipped_letters = 0;
			}
		
			//Single letter substitution starts with 2'b10
			if(i > (STR_LEN - 3)) {
				uint8_t diff = 0x80 | (query.getDatumAt(i));
				retval->push_back(diff);
				i++;
			//Multi-letter substitution starts with 2'b11
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
