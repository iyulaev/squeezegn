#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "Decompressor.h"
#include "utility.h"

Decompressor::Decompressor() {
	;
}

char* Decompressor::decompress_file(const string & compressed_file, Dictionary & dict) const {
	ifstream file_input;
	file_input.open(compressed_file.c_str(), ios::in | ios::binary);
	
	if(!file_input.good()) {
		cerr << "Decompressor could not open input file!" << endl;
		throw(EXCEPTION_FILE_IO);
	}
	
	int uncompressed_file_bytes;
	file_input.read((char*)&uncompressed_file_bytes, sizeof(int));
	
	//Step 1: Allocate output value and initialize it to 'A'
	char* retval = (char*) malloc(sizeof(char) * (uncompressed_file_bytes + 1));
	for(int i = 0; i < uncompressed_file_bytes; i++) { retval[i] = 'A'; }
	
	int number_of_string_inserts;
	file_input.read((char*)&number_of_string_inserts, sizeof(int));
	
	// Step 2: Insert all dictionary indices into output value
	for(int i = 0; i < number_of_string_inserts; i++) {
		int str_pos, str_idx;
		file_input.read((char*)&str_pos, sizeof(int));
		file_input.read((char*)&str_idx, sizeof(int));
		
		char dict_str_buf [STR_LEN+1];
		SequenceWord * dictWord = dict.getWordAt(str_idx);
		dictWord->outputStr(dict_str_buf);
		memcpy(retval + str_pos, dict_str_buf, sizeof(char)*STR_LEN);
	}
	
	//Step 3: Process the "skip / diff list" and modify the output value accordingly
	char* insertion_ptr = retval;
	while(file_input.good()) {
		uint8_t byte_one;
		file_input.read((char*)&byte_one, sizeof(uint8_t));
		
		//printf("Read in char 0x%02x\n", byte_one);
		
		//Begins with 2'b11 -> three-letter insert
		if((byte_one & 0xC0) == 0xC0) {
			for(int i = 0; i < 3; i++) {
				*insertion_ptr = codeToChar(byte_one>>(4-(2*i)));
				insertion_ptr++;
			}
		}
		//Begins with 2'b10 -> single-letter insert
		else if((byte_one & 0xC0) == 0x80) {
			*insertion_ptr = codeToChar(byte_one);
			insertion_ptr++;
		}
		//Begins with 2'b00 -> 14-bit skip
		else {
			uint8_t byte_two;
			file_input.read((char*)&byte_two, sizeof(uint8_t));
			
			int skip_bits = (byte_one << (8*sizeof(uint8_t))) | byte_two;
			insertion_ptr += skip_bits;
		}
	}
	
	retval[uncompressed_file_bytes] = '\0';
	return(retval);
}

int main(int argc, char ** argv) {
	if(argc != 3 && argc != 4) {
		cout << "Invalid number of command line arguments!" << endl;
		cout << "Should be:" << endl;
		cout << "(1) Dictionary file" << endl;
		cout << "(2) Input file" << endl;
		cout << "(3) Dictionary size (optional)" << endl;
	}
	
	string dictFilename(argv[1]);
	
	int dict_size;
	if(argc==4) { dict_size = atoi(argv[3]); }
	else { dict_size = DICTIONARY_SIZE; }
	
	Dictionary dict(dictFilename, dict_size);
	Decompressor engine;
	
	string input_file(argv[2]);
	
	try {
		char* decompressed_string = engine.decompress_file(input_file, dict);
		cout << decompressed_string << endl;
		free(decompressed_string);
	} catch (int e) {
		cerr << "Decompressor exiting with error code " << e << "." << endl;
		return(e);		
	}
	
	return(0);
}
