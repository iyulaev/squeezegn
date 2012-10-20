/** This class is a top-level for a program; it can be used to compress a given file (representing a 
long, concatenated string of short reads) with a given dictionary. Typically we output to stdout */

#include "Compressor.h"

void Compressor::initDataStructures() {
	nPositions = new vector< pair<int,int> > nPositions();
	nPositions = new vector< pair<int,int> > stringPositions();
	diffList = new vector< uint_8t > stringPositions();
}

void Compressor::destroyDataStructures() {
	delete nPositions;
	delete stringPositions;
	delete diffList;
}

void Compressor::clearOutBuffer() {
	//flush internal data structures to file, re-initialize them
	flushDataStructures();
	destroyDataStructures();
	initDataStructures();
}



inline void Compressor::pushSingleCharacter(char c) {
	if(c > 0x60) c -= 0x20;
	
	if(c == 'A') { diffList.push_back(0x80); }
	else if(c == 'C') { diffList.push_back(0x81); }
	else if(c == 'G') { diffList.push_back(0x82); }
	else if(c == 'T') { diffList.push_back(0x83); }
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
				nPositions.push_back( make_pair(file_idx+str_idx, match_idx) ); 
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
	
	file_length_overall += str_idx;
}

int main ( int argc, char ** argv) {
	if(argc != 4) {
		cout << "Invalid number of command-line arguments. Should be three:" << endl;
		cout << "(1) Dictionary file to use." << endl;
		cout << "(2) Genome string to compress." << endl;
		cout << "(3) Output file name." << endl;
		return(-1);
	}
	
	int file_idx = 0;
	char * filebuffer = (char*) malloc(sizeof(char) * (COMPRESSION_MAX_READ_SIZE + 1));
	Compressor engine();
	Dictionary dict(string(argv[1]));
	
	ifstream file_input;
	file_input.open(argv[2]);
	
	if(!file_input.good()) {
		cerr << "Compressor couldn't open file to be compressed." << endl;
		throw EXCEPTION_FILE_IO;
	}
	
	initDataStructures();
	
	while(file_input.good()) {
		file.get(filebuffer, COMPRESSION_MAX_READ_SIZE);
		filerbuffer[COMPRESSION_MAX_READ_SIZE] = '\0';
		string fileString(filebuffer);
		
		//For now let's not worry about replacing Ns
		//file_idx = engine.replaceNs(&nPositions, &fileString, 0);
		engine.compressFileString(dict, fileString, file_idx);
	}
	
	file_input.close();
	
	return(0);
}