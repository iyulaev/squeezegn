#ifndef SQUEEZEGENE_H
#define SQUEEZEGENE_H

#define STR_LEN 128 //better be a multiple of (sizeof(uint64_t) * 8 / 2)!
#define DIFFERENCE_FREE 3
#define DICTIONARY_SIZE 100000

//Exception definitions
#define EXCEPTION_UNKNOWN_ASCII_CHARACTER 900
#define EXCEPTION_UNKNOWN_CODE_CHARACTER 901
#define EXCEPTION_INVALID_ARCHITECTURE 902
#define EXCEPTION_FILE_IO 903

#endif