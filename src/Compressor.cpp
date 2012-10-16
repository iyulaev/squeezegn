/** This class is a top-level for a program; it can be used to compress a given file (representing a 
long, concatenated string of short reads) with a given dictionary. Typically we output to stdout */

#include "Compressor.h"

int Compressor::replaceNs(vector<pair <int,int> >* nPositions, string* fileString, int file_idx) {
	size_t next_N;
	file_idx += fileString.size();
	
	do {
		next_N = fileString->find_first_of('N');
		
		if(next_N != string::npos) {
			
			string::iterator it_nstart = fileString->begin();
			it_nstart += next_N;
			string::iterator it_nend = it_nstart;
			
			while(*it_nend == 'N') { it_nend++; }
			
			int n_length = it_nend - it_nstart;
			nPositions->push_back(make_pair(file_idx, n_length));
			
			fileString->erase(it_nstart, it_nend);
		}
	} while(next_N != string::npos);
	
	return(file_idx);
}

int main ( int argc, char ** argv) {
	if(argc != 3) {
		cout << "Invalid number of command-line arguments. Should be two:" << endl;
		cout << "(1) Dictionary file to use." << endl;
		cout << "(2) Genome string to compress." << endl;
		return(-1);
	}
	
	vector< pair<int, int> >nPositions; //will hold the index and offset of each string of 'N's
	int file_idx = 0;
	char * filebuffer = (char*) malloc(sizeof(char) * (COMPRESSION_MAX_READ_SIZE + 1));
	Compressor engine;
	Dictionary dict(string(argv[1]));
	
	ifstream file_input;
	file_input.open(argv[2]);
	
	if(!file_input.good()) {
		cerr << "Compressor couldn't open file to be compressed." << endl;
		throw EXCEPTION_FILE_IO;
	}
	
	while(file_input.good()) {
		file.get(filebuffer, COMPRESSION_MAX_READ_SIZE);
		filerbuffer[COMPRESSION_MAX_READ_SIZE] = '\0';
		string fileString(filebuffer);
		
		file_idx = engine.replaceNs(&nPositions, &fileString, 0);
	}
	
	file_input.close();
	
	return(0);
}