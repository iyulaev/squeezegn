/** This class is a top-level for a program; it can be used to compress a given file (representing a 
long, concatenated string of short reads) with a given dictionary. Typically we output to stdout */

#include "Compressor.h"
#include <fstream>
#include <exception>
#include <stdlib.h>

#include "utility.h"
#include "SequenceWord.h"

#define DEBUG_COMPRESSOR

Compressor::Compressor() {
	initDataStructures();
}

void Compressor::initDataStructures() {
	nPositions = new vector< pair<int,int> >();
	stringPositions = new vector< pair<int,int> >();
	diffList = new vector< uint8_t >();
}

void Compressor::destroyDataStructures() {
	delete nPositions;
	delete stringPositions;
	delete diffList;
}

void Compressor::flushDataStructures(string & ofile_name) {
	ofstream outfile;
	outfile.open(ofile_name.c_str(), ios::out | ios::binary);
	
	if(!outfile.good()) {
		cerr << "Compressor couldn't open output file " << ofile_name << " for writing." << endl;
		throw EXCEPTION_FILE_IO;
	}
	
	//Write out how many EXACT strings will be output
	int exact_str_count = stringPositions->size();
	outfile.write((const char*) &exact_str_count, sizeof(int));
	
	//write out all stringPositions
	for(auto it = stringPositions->begin(); it < stringPositions->end(); it++) {
		int temp;
		temp = (*it).first;
		outfile.write((const char*) &temp, sizeof(int));
		
		temp = (*it).second;
		outfile.write((const char*) &temp, sizeof(int));
	}
	
	//Write out diffList
	for(auto it = diffList->begin(); it < diffList->end(); it++) {
		uint8_t temp = (*it);
		outfile.write((const char*) &temp, sizeof(uint8_t));
	}
	
	outfile.close();
}

void Compressor::clearOutBuffer(string & ofile_name, int ofile_count) {
	//flush internal data structures to file, re-initialize them
	char file_suffix_buf [5];
	sprintf(file_suffix_buf, "_%03d", ofile_count); //TODO: should be flexible on the number of allowable chunks
	string full_ofile_name = ofile_name + string(file_suffix_buf);
	
	flushDataStructures(full_ofile_name);
	destroyDataStructures();
	initDataStructures();
}



inline void Compressor::pushSingleCharacter(char c) {
	if(c > 0x60) c -= 0x20;
	
	if(c == 'A') { diffList->push_back(0x80); }
	else if(c == 'C') { diffList->push_back(0x81); }
	else if(c == 'G') { diffList->push_back(0x82); }
	else if(c == 'T') { diffList->push_back(0x83); }
}

void Compressor::replaceNs(vector<pair <int,int> >* nPositions, string* fileString, int file_idx) {
	size_t next_N;
	int skipped_ns = 0;
	
	do {
		next_N = fileString->find_first_of('N');
		
		if(next_N != string::npos) {
			string::iterator it_nstart = fileString->begin();
			it_nstart += next_N;
			string::iterator it_nend = it_nstart;
			
			while(*it_nend == 'N') { it_nend++; }
			
			int n_length = it_nend - it_nstart;
			nPositions->push_back(make_pair(file_idx + next_N + skipped_ns, n_length));
			skipped_ns += n_length;
			
			fileString->erase(it_nstart, it_nend);
		}
	} while(next_N != string::npos);
}

void Compressor::compressFileString(Dictionary & dict, string & fileString, int file_idx) {
	int str_idx = 0;
	while(str_idx < fileString.size()) {
		if(fileString.size() - str_idx > STR_LEN) {
			int match_idx = dict.findExactMatch(SequenceWord(fileString.substr(str_idx, str_idx + STR_LEN)));
			
			if(match_idx >= 0) { 
				stringPositions->push_back( make_pair(file_idx+str_idx, match_idx) ); 
				str_idx += STR_LEN;
			}
			else {
				pushSingleCharacter(fileString.at(str_idx++));
			}
		}
		
		else {	
			pushSingleCharacter(fileString.at(str_idx++));
		}
	}
}

int main ( int argc, char ** argv) {
	if(argc != 4) {
		cout << "Invalid number of command-line arguments. Should be three:" << endl;
		cout << "(1) Dictionary file to use." << endl;
		cout << "(2) Genome string to compress." << endl;
		cout << "(3) Output file name." << endl;
		return(-1);
	}
	
	int file_idx = 0; //whan index in the input file we've proceesed through

	//Compressor that is used for compression
	Compressor engine;
	
	//Create (initialize) the Dictionary data structure
	#ifdef DEBUG_COMPRESSOR
	cerr << "Initializing dictionary." << endl;
	#endif
	string dict_filename(argv[1]);
	Dictionary dict(dict_filename, DICTIONARY_SIZE);
	
	//File input
	ifstream file_input;
	file_input.open(argv[2]);
	//Filebuffer that will be used for reading from the input file
	char * filebuffer = (char*) malloc(sizeof(char) * (COMPRESSION_MAX_READ_SIZE + 1));
	
	//Output file name and # of output files
	string ofile_name(argv[3]);
	int ofile_count = 0;
	
	if(!file_input.good()) {
		cerr << "Compressor couldn't open file to be compressed." << endl;
		throw EXCEPTION_FILE_IO;
	}
	
	#ifdef DEBUG_COMPRESSOR
	#endif
	
	#ifdef DEBUG_COMPRESSOR
	cerr << "Opening input file." << endl;
	#endif
	
	while(file_input.good()) {
		file_input.get(filebuffer, COMPRESSION_MAX_READ_SIZE+1);
		string fileString(filebuffer);
		
		//For now let's not worry about replacing Ns
		//file_idx = engine.replaceNs(&nPositions, &fileString, 0);
		engine.compressFileString(dict, fileString, file_idx);
		engine.clearOutBuffer(ofile_name, ofile_count++);
		file_idx += fileString.size();
	}
	
	file_input.close();
	
	return(0);
}