/** Implements a test bench for Dictionary objcet. 

Assumptions: STR_LEN is set to 96
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <exception>

using namespace std;

#include "squeezegene.h"
#include "Dictionary.h"
#include "utility.h"

#define TESTWORD_1 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

#define TESTWORD_2 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAG"

#define TESTWORD_3 "ATAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC"

int main() {
	int errors = 0;

	Dictionary myDict("../test_dict.txt", 4);
	
	SequenceWord testWord(TESTWORD_1);
	
	vector<uint8_t>* diffs = myDict.calcStringDiffs(testWord, 0);
	if(diffs != NULL) {
		for(auto it = diffs->begin(); it < diffs->end(); it++) {
			printf("%02x ", *it);
		}
		
		printf(" - FAIL! [1]\n"); errors++;
	} else {
		printf("PASS! [1]\n");
	}
	
	diffs = myDict.calcStringDiffs(testWord, 1);
	if(diffs != NULL) {
		for(auto it = diffs->begin(); it < diffs->end(); it++) {
			printf("%02x", *it);
		}
		
		if(diffs->size() == 3 && diffs->at(0) == 0x00 && 
			diffs->at(1) == 0x5F && diffs->at(2) == 0x80) {
			printf(" - PASS! [2]\n");
		}
		else {
			printf(" - FAIL! [2]\n");
			errors++;
		}
	} 
		
	
	diffs = myDict.calcStringDiffs(testWord, 3);
	
	if(diffs != NULL) {
		for(auto it = diffs->begin(); it < diffs->end(); it++) {
			printf("%02x", *it);
		}
		
		if(diffs->size() == 3 && diffs->at(1) == 0x00 && 
			diffs->at(2) == 0x5D && diffs->at(0) == 0xC0) {
			printf(" - PASS! [3]\n");
		}
		else {
			printf(" - FAIL! [3]\n");
			errors++;
		}
	} else { printf (" - FAILED! [3]\n"); errors++; }
	
	int match = myDict.findExactMatch(testWord);
	
	if(match != 0) { 
		printf("FAIL - didn't find match (fEM() returned %d) [4]\n", match);
	} else {
		printf("PASS - match (%d) [4]\n", match);
	}
	
	SequenceWord testWord_2(TESTWORD_2);
	match = myDict.findExactMatch(testWord_2);
	
	if(match != 2) { 
		printf("FAIL - didn't find match (fEM() returned %d) [5]\n", match);
	} else {
		printf("PASS - match (%d) [5]\n", match);
	}
	
	
	SequenceWord testWord_3(TESTWORD_3);
	match = myDict.findNearMatch(testWord_3);
	
	if(match != 1) { 
		printf("FAIL - didn't find match (fNM() returned %d) [6]\n", match);
	} else {
		printf("PASS - match (%d) [6]\n", match);
	}
	
	return(0);
}