/** Convert a given genome (A/C/G/T) to binary (0x0, 0x1, ...)
*/

#include <iostream>
#include <stdint.h>
#include <string>
#include <fstream>

using namespace std;

inline uint8_t codeToChar(char letter) {
	switch(letter) {
		case 'A': return(0x0);
		case 'C': return(0x1);
		case 'G': return(0x2);
		case 'T': return(0x3);
	}
}

int main(int argc, char** argv) {
	if(argc != 3) {
		cout << "Invalid number of command line arguments!" << endl;
		cout << "Should be:" << endl;
		cout << "(1) Input ASCII file" << endl;
		cout << "(2) Output BINARY file" << endl;
		
		return(-1);
	}
	
	string inputFilename(argv[1]);
	ifstream file_input;
	file_input.open(inputFilename.c_str(), ios::in);
	
	string outputFilename(argv[2]);
	ofstream file_output;
	file_output.open(outputFilename.c_str(), ios::out | ios::binary );
	
	#define OUTPUT_BUFFER_SZ 16
	char obuf [OUTPUT_BUFFER_SZ];
	for(int x = 0; x < OUTPUT_BUFFER_SZ; x++) {
		obuf[x] = 0;
	}
				
	int	i = 0;
	int	j = 0;
	while(file_input.good()) {
		char input = file_input.get();
		obuf[i] |= (codeToChar(input)<<(j*2));
		j++;
		
		if(4 == j) {
			j = 0;
			i++;
			
			if(OUTPUT_BUFFER_SZ == i) {
				file_output.write(obuf, sizeof(obuf));
				
				for(int x = 0; x < OUTPUT_BUFFER_SZ; x++) {
					obuf[x] = 0;
				}
				
				i = 0;
			}
		}
	}
	
	file_output.write(obuf, sizeof(obuf));
	
	file_input.close();
	file_output.close();
	
	return(0);
}
