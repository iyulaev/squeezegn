#!/bin/bash

# TEST #1
# Only exact matches (and single-character substitutions) should be enabled
../../src/compressor.exe testdata_1_dict.txt testdata_1_input.txt testdata_1_output.hex > /dev/null

if diff -q testdata_1_output.hex_000 testdata_1_output_golden.hex > /dev/null; then
    echo "DictionaryBuilder Test #1 PASS"
else
    echo "DictionaryBuilder Test #1 FAIL"
fi

rm testdata_1_output.hex_000




# TEST #2
# Only exact matches (and single-character substitutions) should be enabled
../../src/compressor.exe testdata_2_dict.txt testdata_2_input.txt testdata_2_output.hex > /dev/null

if diff -q testdata_2_output.hex_000 testdata_2_output_golden.hex > /dev/null; then
    echo "DictionaryBuilder Test #2 PASS"
else
    echo "DictionaryBuilder Test #2 FAIL"
fi

rm testdata_2_output.hex_000





# TEST #3
# Tests that near matches should work
../../src/compressor.exe testdata_3_dict.txt testdata_3_input.txt testdata_3_output.hex > /dev/null

if diff -q testdata_3_output.hex_000 testdata_3_output_golden.hex > /dev/null; then
    echo "DictionaryBuilder Test #3 PASS"
else
    echo "DictionaryBuilder Test #3 FAIL"
fi

rm testdata_3_output.hex_000

echo "DictionaryBuilder Test DONE"
