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
#include <vector>

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
		//This is the ONLY data stored for a sequence word; it is a packed, 2-bit per symbol
		//representation of a sequence (A/C/G/T) word
		uint64_t data [STR_LEN / 4 / sizeof(uint64_t)];
		
		/** Private function; returns the 2-bit code corresponding to the ACGT character provided */
		uint8_t charToCode(char ascii_char) const;
		
		/** Initializes this SequenceWord.data from the provided character string. We assume input_str has length (at least) 
		STR_LEN. This function is used by SequenceWord constructor */
		void initSW(const char * input_str);
		
	public:
		SequenceWord() {;}
		
		/** Create a SequenceWord from a given C string
		The C string should have length at least STR_LEN. */
		SequenceWord(const char *input_str);
		
		/** Create a SequenceWord from a given C++ string
		The C++ string should have length at least STR_LEN. */
		SequenceWord(const string input_str);
		
		/** This is a copy constructor. We create a deep copy of the provided SequenceWord */
		SequenceWord(const SequenceWord& other);
		
		/** Return a pointer to the data (uint64_t array) contained in this SequenceWord */
		const uint64_t* getData(void) const { return(data); }
		
		/** Return the datum (2-bit sequence) located at index (idx) provided */
		uint8_t getDatumAt(int idx) const;
		
		/** Puts a given code datum (2-bit character) into position idx within this SequenceWord's
		data member variable. Used by the constructor to fill this SequenceWord. */
		void pushDatum(uint8_t datum, int idx);
		
		/** Returns an integer representing how many characters (2-bit datums) do not match between this.data and
		other.data. There's some optimization to make this function run as quickly as possible */
		int calcDiff(const SequenceWord & other) const;
		
		/** Returns the index of the first datum that isn't the same between this SW and other 
		If there's no difference, return -1 */
		int firstDatumNotSame(const SequenceWord& other) const;
		
		/** Calculate the difference between this SequenceWord and another SequenceWord and return 
		the list of differences as a uint8_t array. The return value is the difference list (expressed as a
		 vector of bytes) where  if the byte starts with 1'b0, then the next 15 bits represent the number 
		 of sequence symbols we should skip, and if the byte starts with 2'b10, then the next one byte 
		 represents a single symbol replacement and 2'b11 represents three symbol replacements. 
		 
		 Returns NULL if this and other are equal. */
		vector<uint8_t>* calcStringDiffs (const SequenceWord & other) const;
		
		/** Compares two SequenceWords by using bit-wise XOR of data member variables. Returns true if there
		are no difference between the two. */
		bool isEqual(const SequenceWord & other) const;
		
		/** Compares two SequenceWords, returns true if string represented by this.data is less than other.data */
		bool operator<(const SequenceWord & other) const;
		/** Compares two SequenceWords, returns true if string represented by this.data is greater than other.data */
		bool operator>(const SequenceWord & other) const;
		/** Calls isEqual(other). */
		bool operator==(const SequenceWord & other) const;
		
		/** Returns character (ACGT) representation of this.data. Must be given a character buffer of size (at least) STR_LEN. */
		char* outputStr(char* char_buf) const;
};

#endif
