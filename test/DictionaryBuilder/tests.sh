#!/bin/bash

# TEST #1
../../src/DictionaryBuilder.exe testdata_1_input.txt testdata_1_dict_test.txt > /dev/null

if diff -q testdata_1_dict.txt testdata_1_dict_test.txt > /dev/null; then
    echo "DictionaryBuilder Test #1 PASS"
else
    echo "DictionaryBuilder Test #1 FAIL"
fi

rm testdata_1_dict_test.txt

# TEST #2
../../src/DictionaryBuilder.exe testdata_2_input.txt testdata_2_dict_test.txt 4 > /dev/null

if diff -q testdata_2_dict.txt testdata_2_dict_test.txt > /dev/null; then
    echo "DictionaryBuilder Test #2 PASS"
else
    echo "DictionaryBuilder Test #2 FAIL"
fi

rm testdata_2_dict_test.txt

echo "DictionaryBuilder Test DONE"
