#ifndef SQUEEZEGENE_COMPRESSOR
#define SQUEEZEGENE_COMPRESSOR

#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <exception>
#include <vector>

using namespace std;

#include "squeezegene.h"
#include "utility.h"


class Compressor {
	public:
		int replaceNs( vector< pair<int,int> >* nPositions, string* fileString, int file_idx);
};

#endif

