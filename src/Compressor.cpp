/** This class is a top-level for a program; it can be used to compress a given file (representing a 
long, concatenated string of short reads) with a given dictionary. Typically we output to stdout */

#include "Compressor.h"
#include <fstream>
#include <exception>
#include <stdlib.h>

#include "utility.h"
#include "SequenceWord.h"

#define DEBUG_COMPRESSOR
//#define DEBUG_COMPRESSOR_DIFFLIST


#ifdef DEBUG_COMPRESSOR_DIFFLIST
#include <stdio.h>
#endif

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
	
	//Compress the diff list
	//compressDiffs();
	
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

void Compressor::compressDiffs() {
	vector<uint8_t>* new_diffList = new vector< uint8_t >();
	
	//If we're within three bytes of the end we can't look ahead three bytes anymore so compression should stop
	//That's the furthest we might look ahead in the below algorithm
	auto it = diffList->begin();
	while(it < diffList->end()-3) {
		uint8_t read_word = *it;
		
		//Case 1: We have three single-characters in a row, compress them to a two charactrer
		if((read_word & 0xC0) == 0x80) {
			uint8_t read_word_two = *(it+1);
			uint8_t read_word_three = *(it+2);
			
			if((read_word_two & 0xC0) == 0x80 && (read_word_three & 0xC0) == 0x80) {
				uint8_t new_diff = 0xC0 | 
					(read_word_three & 0x3) |
					((read_word_two<<2) & 0xC) |
					((read_word<<4) & 0x30);
					
				new_diffList->push_back(new_diff);
				it+=3;
				
				#ifdef DEBUG_COMPRESSOR_DIFFLIST
				printf("Mashed together three single-chars 0x%02x / 0x%02x / 0x%02x into 0x%02x\n",
						read_word,
						read_word_two,
						read_word_three,
						new_diff);
				#endif
			}
			//No replacement occured - carry on...
			else {
				new_diffList->push_back(read_word);
				it++;
			}
		} else if((read_word & 0xC0) == 0x00) { //Case 2: Match two skips in a row
			uint16_t read_lword = (*it << 8) | (*(it+1));
			uint8_t read_word_two = *(it+2);
			uint16_t read_lword_two = (*(it+2) << 8) | (*(it+3));
			uint16_t max_bit_max = 1 << 13;
			
			//Check if the next word is a skip word, and if the sum of this skip and the next one won't overflow
			//then mash them together
			if((read_word_two & 0xC0) == 0x00 && 
				((read_lword & max_bit_max) == 0) && 
				((read_lword_two & max_bit_max) == 0)) {
				
				uint16_t write_lword = read_lword + read_lword_two;
				new_diffList->push_back((write_lword>>8) & 0xFF);
				new_diffList->push_back(write_lword & 0xFF);
				
				it+=4;
				
				#ifdef DEBUG_COMPRESSOR_DIFFLIST
				printf("Mashed together two skips 0x%04x and 0x%04x into 0x%04x\n",
					read_lword,
					read_lword_two,
					write_lword);
				#endif
				
			} else { //Otherwise just write out the skip
				new_diffList->push_back(*(it++));
				new_diffList->push_back(*(it++));
			}
		} else if((read_word & 0xC0) == 0xC0){ //Other cases - is a three symbol substitution, just push
			new_diffList->push_back(read_word);
			it++;
		} else {
			cerr << "ERROR - compressDiffs() got invalid byte " << read_word << endl;
			it++;
		}
	}
	
	while(it < diffList->end()) {
		new_diffList->push_back(*(it++));
	}
	
	delete diffList;
	diffList = new_diffList;
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
					
					if(mismatch_datum != -1) {
						skipNChars(mismatch_datum);
						pushSingleCharacter(fileString.at(str_idx + mismatch_datum));
						skipNChars(STR_LEN - mismatch_datum - 1);
					} else {
						skipNChars(STR_LEN);
						cerr << "Should never happen : error 1001" << endl;
					}
					
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
	Dictionary dict(dict_filename);
	
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