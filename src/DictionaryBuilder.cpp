/** DictionaryBuilder is used to build a dictionary for the Compressor and Extractor tools, 
part of the squeezegene DNA sequence compression utility 

TODO: modify the output so that we output binary, 2 bits per base Dictionary (1/4 the size)
*/

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <algorithm> //used for (sort)
#include <unordered_map>
#include <exception>

using namespace std;

//For malloc and such
#include <stdlib.h>
//Get stats on file using file system types
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "squeezegene.h"
#include "SequenceWord.h"
#include "utility.h"

class DictionaryBuilder {
	public:	
	vector<SequenceWord>* loadFile(string fileName);
};

#define REMOVE_NS
#define DICTIONARYBUILDER_DEBUG
/** Loads the given file (fileName) into memory, and then generates a vector of SequenceWords, one starting at 
every character of the file. 
*/
vector<SequenceWord>* DictionaryBuilder::loadFile(string fileName) {
	//Determine the length of the file
	struct stat filestatus;
	stat( fileName.c_str(), &filestatus );
	unsigned int file_bytes = filestatus.st_size;
	
	if(file_bytes < STR_LEN) throw EXCEPTION_FILE_IO;
	
	char * file_buffer = (char*) malloc(file_bytes * sizeof(char));
	if(file_buffer == NULL) throw EXCEPTION_FILE_IO;
	
	ifstream myfile;
	myfile.open(fileName.c_str());
	#ifdef REMOVE_NS
	char * file_buf_ptr_remove_ns = file_buffer;
	while(myfile.good()) {
		char temp;
		myfile.get(temp);
		
		if((temp == 'A' || temp == 'C' || temp == 'G' || temp == 'T') && myfile.good()) {
			*(file_buf_ptr_remove_ns++) = temp;
		}
	}
	#else
	if(myfile.good()) {
		myfile.read(file_buffer, file_bytes);
	}
	#endif
	myfile.close();	
	
	#ifdef REMOVE_NS
	int file_strings = file_buf_ptr_remove_ns - file_buffer - STR_LEN + 1;
	#else
	int file_strings = (file_bytes-STR_LEN);
	#endif
	
	vector<SequenceWord>* retval = new vector<SequenceWord>(file_strings);
	char* file_buf_ptr = file_buffer;
	for(auto it = retval->begin(); it < retval->end(); it++) {
		*it = SequenceWord(file_buf_ptr++);
	}
	
	free(file_buffer);
	
	return(retval);
}



int main(int argc, char ** argv)
{
	if(argc != 3) {
		cout << "dictionarybuilder takes two arguments, the file name for short read file \
			to read from, and the dictionary file that we are to generate." << endl;
		return -1;
	}
	
	//http://gcc.gnu.org/onlinedocs/libstdc++/manual/termination.html
	std::set_terminate(__gnu_cxx::__verbose_terminate_handler);

	DictionaryBuilder engine;
	vector<SequenceWord>* sequenceWords;
	
	try {
		cout << "LOADING SRR FILE FROM DISK, TO GENERATE DICTIONARY." << endl;
		sequenceWords = engine.loadFile(argv[1]);
	} catch(int e) {
		cerr << "Threw exception " << e << endl;
		return e;
	}
	
	//Debugging
	#ifdef DICTIONARYBUILDER_DEBUG
	int i = 0;
	for(auto it = sequenceWords->begin(); it < sequenceWords->end(); it++) {
		if(i++ < 5) {
			char char_buf [STR_LEN+1];
			it->outputStr(char_buf);
			cout << "String[" << i << "] = " << char_buf << endl;
			printf("it->data[0] = 0x%ll016x\n", it->getData()[0]);
			
		} else {
			break;
		}
	}
	#endif
	
	cout << endl << "SORTING INITIAL WORD LIST" << endl;
	sort(sequenceWords->begin(), sequenceWords->end());
	cout << endl << "SORTED" << endl;
	
	//Debugging
	#ifdef DICTIONARYBUILDER_DEBUG
	i = 0;
	for(auto it = sequenceWords->begin(); it < sequenceWords->end(); it++) {
		if(i++ < 5) {
			char char_buf [STR_LEN+1];
			it->outputStr(char_buf);
			cout << "String[" << i << "] = " << char_buf << endl;
			printf("it->data[0] = 0x%ll016x\n", it->getData()[0]);
			
		} else {
			break;
		}
	}
	#endif
	
	//Count number of duplicates, debugging
	int num_duplicates = 0;
	unordered_map<string, int> TestMap;
	for(auto it = sequenceWords->begin(); it < sequenceWords->end(); it++) {
		if(it > sequenceWords->begin() && (*it == *(it-1))) {
			char temp_buffer[STR_LEN+1];			
			string dupstring(it->outputStr(temp_buffer));
			
			if(TestMap.find(dupstring) != TestMap.end()) {
				int temp = TestMap[dupstring]+1;
				TestMap.erase(dupstring);
				TestMap[dupstring] = temp;			
			} else {
				TestMap[dupstring] = 1;
				num_duplicates++;
			}
		}
	}
	
	#ifdef DICTIONARYBUILDER_DEBUG
	cout << "Of " << sequenceWords->size() << " strings, found " << \
		num_duplicates << " duplicates!" << endl << endl;
	#endif
	
	vector<pair<SequenceWord, int>> diff_list;
	int strings_processed = 0;
	
	for(auto it = sequenceWords->begin(); it < sequenceWords->end(); it++) {
		int diff = 0;
		
		int i = 0;
		auto it_two = it;
		while((i++) < 50 && (--it_two) >= sequenceWords->begin()) {
			diff+=it->calcDiff(*it_two);
		}
		
		i = 0;
		it_two = it;
		while((i++) < 50 && (++it_two) < sequenceWords->end()) {
			diff+=it->calcDiff(*it_two);
		}
		
		diff_list.push_back(make_pair(*it, diff));
		
		strings_processed++;
		if(strings_processed % 100000 == 0) {
			cout << "Processed " << strings_processed << " strings!" << endl;
		}
	}
	
	sort(diff_list.begin(), diff_list.end(), sort_pairs_by_second<SequenceWord>);
	
	#ifdef DICTIONARYBUILDER_DEBUG
	int j = 0;
	while(diff_list[j].second == 0) { j++; }
	
	for(int i = j; i < j+10 && i < diff_list.size(); i++) {
		char msgbuf[STR_LEN+1];
		diff_list[i].first.outputStr(msgbuf);
		
		cout << "diff_list[" << i << "] = " << msgbuf << endl <<\
			"\t" << diff_list[i].second << endl;
	}
	#endif
	
	ofstream dict_file;
	dict_file.open(argv[2]);
	if(!dict_file.good()) {
		cerr << "Couldn't open dictionary file for writing." << endl;
		throw EXCEPTION_FILE_IO;
	}
	auto it = diff_list.begin();
	for(int i = 0; i < DICTIONARY_SIZE && it < diff_list.end(); i++) {
		char msgbuf[STR_LEN+1];
		(it++)->first.outputStr(msgbuf);
		dict_file << msgbuf << endl;
	}
	dict_file.close();
	
	return 0;
}
