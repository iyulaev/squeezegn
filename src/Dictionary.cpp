#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#include <exception>

#include "squeezegene.h"
#include "utility.h"

#include "Dictionary.h"

using namespace std;

Dictionary::Dictionary(const string & fileName) {
	dictionaryList = loadDictionaryFile(fileName);
	dictionary_size = dictionaryList->size();
}

Dictionary::~Dictionary() {
	if(dictionaryList != NULL) delete dictionaryList;
}

vector<SequenceWord>* Dictionary::loadDictionaryFile(const string & fileName) {
	ifstream myfile;
	vector<SequenceWord>* retval = new vector<SequenceWord>();
	int m_dictionary_size;
	
	myfile.open(fileName.c_str());
	
	if(!myfile.good()) {
		cerr << "Dictionary file could not be opened!" << endl;
		throw EXCEPTION_FILE_IO;
	}
	
	int i = 0;
	while(myfile.good()) {
		char filebuf[STR_LEN+1];
		myfile.getline(filebuf, STR_LEN+1);
		
		//First line indicates size of the dictionary
		if(i == 0) {
			m_dictionary_size = atoi(filebuf);
		}
		else if(strlen(filebuf) >= STR_LEN) {
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

SequenceWord* Dictionary::getWordAt(int idx) const {
	return(new SequenceWord(dictionaryList->at(idx)));
}

vector<uint8_t>* Dictionary::calcStringDiffs(const SequenceWord & query, int target_idx) const {
	SequenceWord target = (*dictionaryList)[target_idx];
	
	#ifdef DICTIONARY_DEBUG_LV1
	printf("calcStringDiffs() comparing the following strings\n");
	char temp[STR_LEN+1];
	query.outputStr(temp);
	printf("Q: %s\n", temp);
	target.outputStr(temp);
	printf("T: %s\n", temp);
	#endif
	
	return(query.calcStringDiffs(target));
}

//#define DEBUG_DICTIONARY_FEM
int Dictionary::findExactMatch(const SequenceWord & input) const {
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


int Dictionary::findFromNearest(const SequenceWord & input) const {
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


int Dictionary::findNearMatch(const SequenceWord & input) const {
	//change each character in input by 1
	for(int i = 0; i < STR_LEN; i++) {
		//there are thre epossible options for each character
		for(int j = 1; j < 4; j++) {
			SequenceWord trial(input);
			trial.pushDatum((trial.getDatumAt(i) + j) % 4, i);
			
			// char msgbuf[256];
			// trial.outputStr(msgbuf);
			// cout << "Going to try to find exact match for " << msgbuf << endl;
			
			int position = findExactMatch(trial);
			
			if(position != -1) { return(position); }
		}
	}
	
	return(-1);
}
