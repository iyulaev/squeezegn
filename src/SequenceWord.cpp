#include <stdio.h>
#include "sequenceword.h"

/** Private function; returns the 2-bit code corresponding to the ACGT character provided */
uint8_t SequenceWord::charToCode(char ascii_char) const {
	//Convert input to upper case if necessary
	if(ascii_char > 0x5A) { ascii_char -= 0x20; }
	
	switch(ascii_char) {
		case 'A': return(0x0);
		case 'C': return(0x1);
		case 'G': return(0x2);
		case 'T': return(0x3);
	}
	
	#ifdef DEBUG
		cout << "Got unknown character " << ascii_char << endl;
	#endif
	printf("Got unknown character %c (0x%02x)\n", ascii_char, ascii_char);
	throw EXCEPTION_UNKNOWN_ASCII_CHARACTER;
}

/** Opposite of charToCode() returns ACGT character for the 2-bit code provided */
char SequenceWord::codeToChar(uint8_t code) const {
	switch(code) {
		case 0x0: return('A');
		case 0x1: return('C');
		case 0x2: return('G');
		case 0x3: return('T');
	}
	
	#ifdef DEBUG
		cout << "Got unknown code " << code << endl;
	#endif
	throw EXCEPTION_UNKNOWN_CODE_CHARACTER;
}

/** Puts a given code datum (2-bit character) into position idx within this SequenceWord's
data member variable. Used by the constructor to fill this SequenceWord. */
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

/** Return the datum (2-bit sequence) located at index (idx) provided */
uint8_t SequenceWord::getDatumAt(int idx) {
	int idx_word = idx / (sizeof(uint64_t)*8/2);
	int idx_pos = 2 * (idx % (sizeof(uint64_t)*8/2));
	
	uint64_t datumWord = data[idx_word];
	return((uint8_t)((datumWord >> idx_pos) & 0x3ll));
}

/** Initializes this SequenceWord.data from the provided character string. We assume input_str has length (at least) 
STR_LEN. This function is used by SequenceWord constructor */
void SequenceWord::initSW(const char * input_str) {
	for(int i = 0; i < STR_LEN * 2 / sizeof(uint64_t); i++) {
		data[i] = 0x0;
	}
	
	for(int i = 0; i < STR_LEN; i++) {
		pushDatum(charToCode(input_str[i]), i);
	}
}

/** Create a SequenceWord from a given C string
The C string should have length at least STR_LEN. */
SequenceWord::SequenceWord(const char * input_str) {
	initSW(input_str);
}

/** Create a SequenceWord from a given C++ string
The C++ string should have length at least STR_LEN. */
SequenceWord::SequenceWord(const string input_str) {
	initSW(input_str.c_str());
}

/** This is a copy constructor. We create a deep copy of the provided SequenceWord */
SequenceWord::SequenceWord(const SequenceWord& other) {
	const uint64_t* other_data = other.getData();
	
	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		data[i] = other_data[i];
	}
}

/** Returns an integer representing how many characters (2-bit datums) do not match between this.data and
other.data. There's some optimization to make this function run as quickly as possible */
int SequenceWord::calcDiff(SequenceWord other) const {
	//We assume that unsigned long longs are 64-bits, otherwse the popcount call won't work
	if(sizeof(unsigned long long) != sizeof(uint64_t)) {
		#ifdef DEBUG
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

/** Compares two SequenceWords by using bit-wise XOR of data member variables. Returns true if there
are no difference between the two. */
bool SequenceWord::isEqual(SequenceWord other) const {
	const uint64_t* other_data = other.getData();
	
	for(int i = 0; i < (STR_LEN*2) / (sizeof(uint64_t) * 8); i++) {
		if((data[i] ^ other_data[i]) != 0x0ll) { return false; }
	}
	
	return true;
}

/** Compares two SequenceWords, returns true if string represented by this.data is less than other.data */
bool SequenceWord::operator<(SequenceWord other) const {
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

/** Compares two SequenceWords, returns true if string represented by this.data is greater than other.data */
bool SequenceWord::operator>(SequenceWord other) const {
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

/** Calls isEqual(other). */
bool SequenceWord::operator==(SequenceWord other) const {
	return(isEqual(other));
}

/** Returns character (ACGT) representation of this.data. Must be given a character buffer of size (at least) STR_LEN. */
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


#ifdef DEBUG
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
