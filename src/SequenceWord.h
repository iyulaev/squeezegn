/** SequenceWord defines the basic "word" for sequence data that gets processed with
SqueezeGene. We convert and store a sequence string, of length STR_LEN, as a packed
array, with two bits representing {A,C,G,T}. Bitwise operations can then be used to compare 
SequenceWords. Makes things efficient.

Documentation for the functions may be found in SequenceWord.cpp

Written I. Yulaev 2012-10-03
*/

#ifndef INCLUDE_SEQUENCEWORD
#define INCLUDE_SEQUENCEWORD

#include <string>
#include <stdint.h>

//Enable debugging output
//#define DEBUG_SEQUENCEWORD
#ifdef DEBUG_SEQUENCEWORD
#include <iostream>
#include <stdio.h>
#endif

#include "squeezegene.h"

using namespace std;

class SequenceWord {
	private:
		uint64_t data [STR_LEN / 4 / sizeof(uint64_t)];
		
		uint8_t charToCode(char ascii_char) const;
		char codeToChar(uint8_t code) const;
		void initSW(const char * input_str);
		
	public:
		SequenceWord() {;}
		SequenceWord(const char *input_str);
		SequenceWord(const string input_str);
		SequenceWord(const SequenceWord& other);
		const uint64_t* getData(void) const { return(data); }
		uint8_t getDatumAt(int idx);
		void pushDatum(uint8_t datum, int idx);
		
		int calcDiff(SequenceWord other) const;
		int firstDatumNotSame(const SequenceWord& other) const;
		bool isEqual(SequenceWord other) const;
		bool operator<(SequenceWord other) const;
		bool operator>(SequenceWord other) const;
		bool operator==(SequenceWord other) const;
		
		char* outputStr(char* char_buf) const;
};

#endif
