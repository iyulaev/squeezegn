/** The decompressor is responsible for re-creating the string represented by the compressed file. A reference dictionary 
(that matches what was used during compression, of course) is required. */

#ifndef SQUEEZEGENE_DECOMPRESSOR
#define SQUEEZEGENE_DECOMPRESSOR

#include <string>
#include <vector>

using namespace std;

#include "Dictionary.h"

class Decompressor{ 

	public:
	Decompressor();
	char* decompress_file(const string & compressed_file, Dictionary & dict);
};

#endif
