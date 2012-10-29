#include <stdio.h>
#include "sequenceword.h"
#include "utility.h"

uint8_t SequenceWord::charToCode(char ascii_char) const {
	//Convert input to upper case if necessary
	if(ascii_char > 0x5A) { ascii_char -= 0x20; }
	
	switch(ascii_char) {
		case 'A': return(0x0);
		case 'C': return(0x1);
		case 'G': return(0x2);
		case 'T': return(0x3);
	}
	
	#ifdef DEBUG_SEQUENCEWORD
	printf("Got unknown character %c (0x%02x)\n", ascii_char, ascii_char);
	#endif
	throw EXCEPTION_UNKNOWN_ASCII_CHARACTER;
}

/** Opposite of charToCode() returns ACGT character for the 2-bit code provided */
/*char SequenceWord::codeToChar(uint8_t code) const {
	switch(code) {
		case 0x0: return('A');
		case 0x1: return('C');
		case 0x2: return('G');
		case 0x3: return('T');
	}
	
	#ifdef DEBUG_SEQUENCEWORD
		cout << "Got unknown code " << code << endl;
	#endif
	throw EXCEPTION_UNKNOWN_CODE_CHARACTER;
}*/

void SequenceWord::pushDatum(uint8_t datum, int idx) {
	uint64_t datum_big = (uint64_t) datum;
	uint64_t DATUM_MASK = 0x3ll;
	
	int idx_word = idx / (sizeof(uint64_t)*8/2);
	int idx_pos = 2 * (idx % (sizeof(uint64_t)*8/2));
	uint64_t old_data = data[idx_word];
	uint64_t mask = 0xFFFFFFFFFFFFFFFFll ^ (DATUM_MASK << idx_pos);
	old_data = (old_data & mask);
	data[idx_word] = old_data | ((datum_big&DATUM_MASK) << idx_pos);
}

uint8_t SequenceWord::getDatumAt(int idx) const {
	int idx_word = idx / (sizeof(uint64_t)*8/2);
	int idx_pos = 2 * (idx % (sizeof(uint64_t)*8/2));
	
	uint64_t datumWord = data[idx_word];
	return((uint8_t)((datumWord >> idx_pos) & 0x3ll));
}

void SequenceWord::initSW(const char * input_str) {
	for(int i = 0; i < STR_LEN / 4 / sizeof(uint64_t); i++) {
		data[i] = 0x0;
	}
	
	for(int i = 0; i < STR_LEN; i++) {
		pushDatum(charToCode(input_str[i]), i);
	}
}


SequenceWord::SequenceWord(const char * input_str) {
	initSW(input_str);
}


SequenceWord::SequenceWord(const string input_str) {
	initSW(input_str.c_str());
}


SequenceWord::SequenceWord(const SequenceWord& other) {
	const uint64_t* other_data = other.getData();
	
	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		data[i] = other_data[i];
	}
}


int SequenceWord::calcDiff(const SequenceWord & other) const {
	//We assume that unsigned long longs are 64-bits, otherwse the popcount call won't work
	if(sizeof(unsigned long long) != sizeof(uint64_t)) {
		#ifdef DEBUG_SEQUENCEWORD
			cerr << "Error, ULL != uint64_t!" << endl;
		#endif
		throw EXCEPTION_INVALID_ARCHITECTURE;
	}

	const uint64_t* other_data = other.getData();
	
	//XOR the two data words together
	uint64_t xor_result [STR_LEN*2 / (sizeof(uint64_t) * 8)];
	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		xor_result[i] = data[i] ^ other_data[i];
	}
	
	//Compress the bits so that for every 2-bit datum, we have 0x01 if EITHER of the previous bits were high.
	uint64_t diff_counts [STR_LEN*2 / (sizeof(uint64_t) * 8)];
	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t)*8); i++) {
		diff_counts[i] = ((xor_result[i]& 0x5555555555555555ll)) | ((xor_result[i]&0xAAAAAAAAAAAAAAAAll)>>1);
	}
	
	//Count the number of bits that are set high
	int retval = 0;
	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t)*8); i++) {
		retval += __builtin_popcountll((unsigned long long) diff_counts[i]);
	}
	
	return(retval);
}

/*TODO: make this more efficient (hand-manipulation of bits and the like)
*/
int SequenceWord::firstDatumNotSame(const SequenceWord & other) const {
	const uint64_t* other_data = other.getData();
	
	for(int i = 0; i < (STR_LEN/4)/sizeof(uint64_t); i++) {
		if(data[i] != other_data[i]) {
			uint8_t* this_data_bytes = (uint8_t*) data;
			uint8_t* other_data_bytes = (uint8_t*) other_data;
			
			for(int j = 0; j < sizeof(uint64_t)/sizeof(uint8_t); j++) {
				if(this_data_bytes[j] != other_data_bytes[j]) {
					uint8_t this_byte = this_data_bytes[j];
					uint8_t other_byte = other_data_bytes[j];
					
					int bit_broken = -1;
					
					if((this_byte&0x03) != (other_byte & 0x03)) { bit_broken = 0; }
					else if((this_byte&0x0C) != (other_byte & 0x0C)) { bit_broken = 1; }
					else if((this_byte&0x30) != (other_byte & 0x30)) { bit_broken = 2; }
					else if((this_byte&0xC0) != (other_byte & 0xC0)) { bit_broken = 3; }
					
					return(bit_broken + j*(8/2) + i*(sizeof(uint64_t) * (8/2)));
				}
			}
		}
	}
	
	return(-1);
}


vector<uint8_t>* SequenceWord::calcStringDiffs (const SequenceWord & other) const {
	if(isEqual(other)) { return(NULL); }
	
	vector<uint8_t>* retval = new vector<uint8_t>();
	uint16_t skipped_letters = 0;
	
	int i = 0;
	while(i < STR_LEN) {
		if(getDatumAt(i) != other.getDatumAt(i)) {
			#ifdef DICTIONARY_DEBUG_LV1
			fprintf(stderr, "Determined that datums 0x%02x and 0x%02x were unequal!\n",
				getDatumAt(i),
				other.getDatumAt(i));
			#endif
		
			//Skip letter (16 bit sequence) starts with 1'b0
			if(skipped_letters > 0) {
				uint16_t skipword = skipped_letters & 0x7FFF;
				retval->push_back((uint8_t)((skipword >> 8) & 0xFF));
				retval->push_back((uint8_t) (skipword & 0xFF));
				skipped_letters = 0;
			}
		
			//Single letter substitution starts with 2'b10
			if(i > (STR_LEN - 3)) {
				uint8_t diff = 0x80 | (getDatumAt(i));
				retval->push_back(diff);
				i++;
			//Multi-letter substitution starts with 2'b11
			} else {
				uint8_t diff = 0xC0 | ((getDatumAt(i)<<4)|(getDatumAt(i+1)<<2)|(getDatumAt(i+2)));
				retval->push_back(diff);
				i+=3;
			}
		}
		
		else {
			i++;
			skipped_letters++;
		}
	}
	
	if(skipped_letters > 0) {
		uint16_t skipword = skipped_letters & 0x7FFF;
		retval->push_back((uint8_t)((skipword >> 8) & 0xFF));
		retval->push_back((uint8_t) (skipword & 0xFF));
		skipped_letters = 0;
	}
	
	return(retval);
}

bool SequenceWord::isEqual(const SequenceWord & other) const {
	const uint64_t* other_data = other.getData();
	
	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		if((data[i] ^ other_data[i]) != 0x0ll) { return false; }
	}
	
	return true;
}

bool SequenceWord::operator<(const SequenceWord & other) const {
	const uint64_t* other_data = other.getData();

	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		for(int j = 0; j < (sizeof(uint64_t) * 8); j+=2) {
			if( ((data[i]>>j) & 0x3ll) < ((other_data[i]>>j) & 0x3ll) ) {
				return true;
			} else if ( ((data[i]>>j) & 0x3ll) > ((other_data[i]>>j) & 0x3ll) ) {
				return false;
			}
		}
	}
	
	return false;
}

bool SequenceWord::operator>(const SequenceWord & other) const {
	const uint64_t* other_data = other.getData();

	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		for(int j = 0; j < (sizeof(uint64_t) * 4); j++) {
			if( ((data[i]>>j) & 0x3ll) > ((other_data[i]>>j) & 0x3ll) ) {
				return true;
			} else if ( ((data[i]>>j) & 0x3ll) < ((other_data[i]>>j) & 0x3ll) ) {
				return false;
			}
		}
	}
	
	return false;
}

bool SequenceWord::operator==(const SequenceWord & other) const {
	return(isEqual(other));
}

char * SequenceWord::outputStr(char* char_buf) const {
	char* char_buf_ptr = char_buf;	

	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		for(int j = 0; j < (sizeof(uint64_t) * 4); j++) {
			*(char_buf_ptr++) = codeToChar((data[i]>>(j*2)) & 0x3ll);
		}
	}
	
	*char_buf_ptr = '\0';

	return char_buf;
}


#ifdef DEBUG_SEQUENCEWORD
int main() {
	SequenceWord test("ACGTACGTACGTACGTACGTACGTACGTACGTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	SequenceWord test2("AAGTACGTACGTACGTCCGTACGTACGTACGTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	const uint64_t* data = test.getData();
	
	printf("Test: 0x%ll016x\n", data[0]);
	printf("Test: 0x%ll016x\n", data[1]);
	
	printf("Distance between 0 and 2 is %d\n", test.calcDiff(test2));
	
	if(test2 < test) { 
		cout << "test2 < test" << endl;
	}
	
	if(!(test2 > test)) { 
		cout << "! test2 < test" << endl;
	}
	
	char char_buf[STR_LEN+1];
	test.outputStr(char_buf);
	cout << "Test (string): " << char_buf << endl;
	
	return 0;
}
#endif
