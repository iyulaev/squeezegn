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

Compressor::~Compressor() {
	destroyDataStructures();
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

void Compressor::flushDataStructures(const string & ofile_name, int chars_compressed) {
	ofstream outfile;
	outfile.open(ofile_name.c_str(), ios::out | ios::binary | ios::trunc);
	
	if(!outfile.good()) {
		cerr << "Compressor couldn't open output file " << ofile_name << " for writing." << endl;
		throw EXCEPTION_FILE_IO;
	}
	
	//Write out the uncompressed file length
	outfile.write((const char*) &chars_compressed, sizeof(int));
	
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

int Compressor::clearOutBuffer(const string & ofile_name, int ofile_count, int chars_compressed) {
	//flush internal data structures to file, re-initialize them
	char file_suffix_buf [5];
	sprintf(file_suffix_buf, "_%03d", ofile_count); //TODO: should be flexible on the number of allowable chunks
	string full_ofile_name = ofile_name + string(file_suffix_buf);
	
	#ifdef DEBUG_COMPRESSOR
	cerr << "Writing output file: " << full_ofile_name << endl;
	#endif
	
	try {
		flushDataStructures(full_ofile_name, chars_compressed);
	} catch (int e) {
		cerr << "clearOutBuffer() couldn't flush data structures to file!" << endl;
		return(1);
	}
	destroyDataStructures();
	initDataStructures();
	
	return(0);
}



inline void Compressor::pushSingleCharacter(char c) {
	if(c > 0x60) c -= 0x20;
	
	if(c == 'A') { diffList->push_back(0x80); }
	else if(c == 'C') { diffList->push_back(0x81); }
	else if(c == 'G') { diffList->push_back(0x82); }
	else if(c == 'T') { diffList->push_back(0x83); }
}

inline void Compressor::skipNChars(int skip_positions) {
	uint16_t skips = (uint16_t) skip_positions;
	diffList->push_back(((uint8_t*)&skips)[1]);
	diffList->push_back(((uint8_t*)&skips)[0]);
}

void Compressor::replaceNs(vector<pair <int,int> >* nPositions, string & fileString, int file_idx) {
	size_t next_N;
	int skipped_ns = 0;
	
	do {
		next_N = fileString.find_first_of('N');
		
		if(next_N != string::npos) {
			string::iterator it_nstart = fileString.begin();
			it_nstart += next_N;
			string::iterator it_nend = it_nstart;
			
			while(*it_nend == 'N') { it_nend++; }
			
			int n_length = it_nend - it_nstart;
			nPositions->push_back(make_pair(file_idx + next_N + skipped_ns, n_length));
			skipped_ns += n_length;
			
			fileString.erase(it_nstart, it_nend);
		}
	} while(next_N != string::npos);
}

#define ENABLE_ONESUB_MATCH
#define ENABLE_NEARRADIUS_MATCH
int Compressor::compressFileString(const Dictionary & dict, const string & fileString) {
	int str_idx = 0;
	while(str_idx < fileString.size()) {
		//Try to find exact match
		if(fileString.size() - str_idx > STR_LEN) {
			int exact_match_idx = dict.findExactMatch(SequenceWord(fileString.substr(str_idx, STR_LEN)));
			
			if(exact_match_idx >= 0) { 
				stringPositions->push_back( make_pair(str_idx, exact_match_idx) ); 
				skipNChars(STR_LEN);
				str_idx += STR_LEN;
			}
			
			//Try to find match for single-character substitution
			#ifdef ENABLE_ONESUB_MATCH
			else {
				int near_match_idx = dict.findNearMatch(SequenceWord(fileString.substr(str_idx, str_idx + STR_LEN)));
				
				if(near_match_idx >= 0) { 
					stringPositions->push_back( make_pair(str_idx, near_match_idx) ); 
					
					SequenceWord queryWord(fileString.substr(str_idx, STR_LEN));
					SequenceWord* targetWord = dict.getWordAt(near_match_idx);
					int mismatch_datum=queryWord.firstDatumNotSame(*targetWord);
					delete targetWord;
					
					skipNChars(mismatch_datum);
					pushSingleCharacter(fileString.at(str_idx + mismatch_datum));
					skipNChars(STR_LEN - mismatch_datum - 1);
					
					str_idx += STR_LEN;
				}
				
				//Try to find match from nearest "search radius"
				#define MAX_DIFF_COUNT 3
				#ifdef ENABLE_NEARRADIUS_MATCH
				else { 
					SequenceWord queryWord(fileString.substr(str_idx, STR_LEN));
					
					int near_match_idx = dict.findFromNearest(queryWord);
					int diffs_w_near_match = -1;
					if(near_match_idx >= 0) {
						SequenceWord* targetWord = dict.getWordAt(near_match_idx);
						diffs_w_near_match = queryWord.calcDiff(*targetWord);
					}
					
					if(diffs_w_near_match >= 0 && diffs_w_near_match <= MAX_DIFF_COUNT) {
						
						//make compressorcout << "Compressor doing nearest radius insert." << endl;
						
						stringPositions->push_back( make_pair(str_idx, near_match_idx) );

						char msgbuf[256];
						SequenceWord* targetWord = dict.getWordAt(near_match_idx);
						targetWord->outputStr(msgbuf);
						// printf("Diffing strings %s and ", msgbuf);
						// cout << fileString.substr(str_idx, STR_LEN) << endl;
						vector<uint8_t>* dictDiffList = dict.calcStringDiffs(
							queryWord, 
							near_match_idx);
							
						for(auto it = dictDiffList->begin(); it < dictDiffList->end(); it++) {
							diffList->push_back(*it);
							//printf("Pushing diff list item 0x%02x\n", *it);
						}
						
						delete dictDiffList;
						str_idx += STR_LEN;
					}
					
					else { 
						// printf("Pushing single character %c to output pos %d\n",
							// fileString.at(str_idx),
							// str_idx);
						pushSingleCharacter(fileString.at(str_idx++));
					}
				}
				#else
				else { 
					pushSingleCharacter(fileString.at(str_idx++));
				}
				#endif
			}
			#else
			else { 
				pushSingleCharacter(fileString.at(str_idx++));
			}
			#endif
		}
		
		//If we don't have enough characters left in the string we have no choice but to push them to the end
		else {	
			// printf("Pushing (OOC) single character %c to output pos %d\n",
							// fileString.at(str_idx),
							// str_idx);
			pushSingleCharacter(fileString.at(str_idx++));
		}
	}
	
	return(str_idx);
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
	cerr << "Opening input file." << endl;
	#endif
	
	while(file_input.good()) {
		file_input.get(filebuffer, COMPRESSION_MAX_READ_SIZE+1);
		string fileString(filebuffer);
		
		//For now let's not worry about replacing Ns
		//file_idx = engine.replaceNs(&nPositions, &fileString, 0);
		int chars_compressed = engine.compressFileString(dict, fileString);
		int cob_retval = engine.clearOutBuffer(ofile_name, ofile_count++, chars_compressed);
		file_idx += chars_compressed;
		
		if(cob_retval != 0) break;
	}
	
	file_input.close();
	
	#ifdef DEBUG_COMPRESSOR
	cerr << "Done writing files" << endl;
	#endif
	
	return(0);
}