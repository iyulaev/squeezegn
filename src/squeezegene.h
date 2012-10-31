#ifndef SQUEEZEGENE_H
#define SQUEEZEGENE_H

#define STR_LEN 96 //better be a multiple of (sizeof(uint64_t) * 8 / 2)!
#define DIFFERENCE_FREE 3
#define DICTIONARY_SIZE 1000

/* Define how many characters to read from the uncompressed file at a time. Larger reads will place greater 
demands on system memory but will make compression slightly more efficient due to compression boundaries 
being more sparse */
#define COMPRESSION_MAX_READ_SIZE 100000

//Define the search radius for nearest-sorted-match searches
#define DICTIONARY_NEAREST_SEARCH_RADIUS 50

//Exception definitions
#define EXCEPTION_UNKNOWN_ASCII_CHARACTER 900
#define EXCEPTION_UNKNOWN_CODE_CHARACTER 901
#define EXCEPTION_INVALID_ARCHITECTURE 902
#define EXCEPTION_FILE_IO 903

#endif
