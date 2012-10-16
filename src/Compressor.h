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
		int file_length_overall;
		int file_length_no_ns;
		
		vector< pair<int,int> >* nPositions;
		vector< pair<int,int> >* stringPositions;
		vector<uint_8t>* diffList;
		
		void compressDiffList(vector<uint_8t>* diffList);
		void clearOutBuffer();
	public:
		int replaceNs( vector< pair<int,int> >* nPositions, string* fileString, int file_idx);
		void compressFile(string dictFileName, string inputFileName, string outputFileName);
};

#endif
