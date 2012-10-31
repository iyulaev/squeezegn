#ifndef SQUEEZEGENE_COMPRESSOR
#define SQUEEZEGENE_COMPRESSOR

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

#include "squeezegene.h"
#include "Dictionary.h"

class Compressor {
	private:
		//Stores positions and lengths of N sequences
		vector< pair<int,int> >* nPositions;
		//Stores positions and dictionary indices of "exact matches" from dictionary
		vector< pair<int,int> >* stringPositions;
		//Stores diffs between the exact strings and the target file output
		//Format is:
		//starting with 2'b10 - single-base substitution
		//Starting with 2'b11 - three-base substitutions (three bases are packed into the lower-most 6 bits)
		//starting with 1'b0 - next 15 bits are a skip
		vector<uint8_t>* diffList;
		
		/** Initialize data structures for a single compression run */
		void initDataStructures();
		/** Free data structures */
		void destroyDataStructures();
		/** Flush data structures to output file (with output file name provided) */
		void flushDataStructures(const string & ofile_name, int chars_compressed);
		/** Compress the diff list:
			Any three single-bit substitutions turn into a single three-bit substitution
			Any two adjacent skip words are combined into a single skip word - (if we don't overflow of course!) 
		*/
		void compressDiffs();
		
		/** Push a single character to diffList */
		inline void pushSingleCharacter(char c);
		/** Push a single skip entry into diffList */
		inline void skipNChars(int skip_positions);
		
	public:
		/** Creates a new Compressor */
		Compressor();
		~Compressor();
		
		/** Removes all strings of 'N' characters in fileString, and adds to nPositions the position and length
			of each extracted 'N' string
			file_idx simply tells the function where in the UNCOMPRESSED, UNALTERED file fileString is located
		*/
		void replaceNs( vector< pair<int,int> >* nPositions, string & fileString, int file_idx);
		
		/** Flush the output buffer to file 
			Takes name of output file, the file # (input "chunk") that we have finished processing, and the uncompressed character
			count of the data that we flush out. 
			
			Returns 0 if clearing output buffers was successful.
			*/
		int clearOutBuffer(const string & ofile_name, int ofile_count, int chars_compressed);
		
		/** Compress a given file string, putting the exact matches (relative to the dictionary) into stringPositions
			and putting substiutions into diffList. Note that compressDiffList() should probably be run before
			pushing the diffList to file as this funciton is not terribly efficient at writing out a solid diff list 
			
			file_idx is where the string begins in the original uncompressed unaltered file
			
			returns the length of the UNCOMPRESSED string that we have compressed
		*/
		int compressFileString(const Dictionary & dict, const string & fileString);
};

#endif
