#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <exception>

using namespace std;


#include "squeezegene.h"
#include "SequenceWord.h"
#include "utility.h"

class Dictionary {
	private:	
	vector<SequenceWord>* loadFile(string fileName);
};

/** Loads the given dictionaryfile into a vector of SequenceWords
*/
vector<SequenceWord>* DictionaryBuilder::loadDictionaryFile(string fileName) {
	ifstream myfile;
	vector<SequenceWord>* retval = new vector<SequenceWord>(DICT_SIZE);
	auto it = retval->begin();
	
	myfile.open(fileName.c_str());
	while(myfile.good()) {
		char filebuf[STR_LEN+1];
		myfile.getline(filebuf, STR_LEN+1);
		*(it++) = SequenceWord(filebuf);
	}
	myfile.close();	
		
	return(retval);
}
