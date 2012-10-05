/** This class is a top-level for a program; it can be used to compress a given file (representing a 
long, concatenated string of short reads) with a given dictionary. Typically we output to stdout */

#include "Compressor.h"

int Compressor::replaceNs(vector<pair <int,int> >* nPositions, string* fileString, int file_idx) {
	size_t next_N;
	do {
		next_N = fileString->find_first_of('N');
		
		if(next_N != string::npos) {
			file_idx += next_N;
			
			string::iterator it_nstart = fileString->begin();
			it_nstart += next_N;
			string::iterator it_nend = it_nstart;
			
			while(*it_nend == 'N') { it_nend++; }
			
			int n_length = it_nend - it_nstart;
			nPositions->push_back(make_pair(file_idx, n_length));
			file_idx += n_length;
			
			fileString->erase(it_nstart, it_nend);
		}
	} while(next_N != string::npos);
	
	return(file_idx);
}

int main ( int argc, char ** argv) {
	vector< pair<int, int> >nPositions; //will hold the index and offset of each string of 'N's
	int file_idx;
	
	Compressor engine;
	
	string fileString("ABCDNNNN");
	file_idx = engine.replaceNs(&nPositions, &fileString, 0);
	
	cout << "After processing, string was " << fileString << ", file_idx = " << file_idx <<\
		", nPositions[0] = (" << nPositions[0].first << ", " << nPositions[0].second << ")" << endl;
		
	return(0);
}