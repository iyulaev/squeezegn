#ifndef SQUEEZEGENE_COMPRESSOR
#define SQUEEZEGENE_COMPRESSOR

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <exception>
#include <vector>

using namespace std;

#include "squeezegene.h"
#include "utility.h"
#include "SequenceWord.h"
#include "Dictionary.h"


class Compressor {
	private:
		//Overall UNCOMPRESSED file length
		int file_length_overall;
		//Overall UNCOMPRESSED file length with all strings of 'N' characters removed
		int file_length_no_ns;
		
		//Stores positions and lengths of N sequences
		vector< pair<int,int> >* nPositions;
		//Stores positions and lengths of "exact matches" from dictionary
		vector< pair<int,int> >* stringPositions;
		//Stores diffs between the exact strings and the target file output
		//Format is:
		//starting with 2'b10 - single-base substitution
		//Starting with 2'b11 - three-base substitutions (three bases are packed into the lower-most 6 bits)
		vector<uint_8t>* diffList;
		
		/** Initialize data structures for a single compression run */
		void initDataStructures();
		/** Free data structures */
		void destroyDataStructures();
		
		/** Push a single character to diffList */
		inline void pushSingleCharacter(char c);
		
		/** Compress the diff list:
			Any three single-bit substitutions turn into a single three-bit substitution
			Any two adjacent skip words are combined into a single skip word - (if we don't overflow of course!) 
		*/
		void compressDiffList(vector<uint_8t>* diffList);
		/** Flush the output buffer to file */
		void clearOutBuffer();
	public:
		/** Removes all strings of 'N' characters in fileString, and adds to nPositions the position and length
			of each extracted 'N' string
			file_idx simply tells the function where in the UNCOMPRESSED, UNALTERED file fileString is located
		*/
		int replaceNs( vector< pair<int,int> >* nPositions, string* fileString, int file_idx);
		
		/** Compress a given file string, putting the exact matches (relative to the dictionary) into stringPositions
			and putting substiutions into diffList. Note that compressDiffList() should probably be run before
			pushing the diffList to file as this funciton is not terribly efficient at writing out a solid diff list 
			
			file_idx is where the string begins in the original uncompressed unaltered file
		*/
		int compressFileString(Dictionary & dict, string & fileString, int file_idx);
};

#endif
