#!/bin/bash

# TEST #1
# Only exact matches (and single-character substitutions) should be enabled
../../src/decompressor.exe testdata_1_dict.txt testdata_1_output_golden.hex 6 > testdata_1_decompressed.txt

if diff -qw testdata_1_input.txt testdata_1_decompressed.txt > /dev/null; then
    echo "Decompressor Test #1 PASS"
	rm testdata_1_decompressed.txt
else
    echo "Decompressor Test #1 FAIL"
fi



# TEST #2
# Only exact matches (and single-character substitutions) should be enabled
../../src/decompressor.exe testdata_2_dict.txt testdata_2_output_golden.hex 4 > testdata_2_decompressed.txt

if diff -qw testdata_2_input.txt testdata_2_decompressed.txt > /dev/null; then
    echo "Decompressor Test #2 PASS"
	rm testdata_2_decompressed.txt
else
    echo "Decompressor Test #2 FAIL"
fi




# TEST #3
# Tests that near matches should work
../../src/decompressor.exe testdata_3_dict.txt testdata_3_output_golden.hex 1 > testdata_3_decompressed.txt

if diff -qw testdata_3_input.txt testdata_3_decompressed.txt > /dev/null; then
    echo "Decompressor Test #3 PASS"
	rm testdata_3_decompressed.txt
else
    echo "Decompressor Test #3 FAIL"
fi





# TEST #4
# Tests that near matches should work
../../src/decompressor.exe testdata_4_dict.txt testdata_4_output_golden.hex 4 > testdata_4_decompressed.txt

if diff -qw testdata_4_input.txt testdata_4_decompressed.txt > /dev/null; then
    echo "Decompressor Test #4 PASS"
	rm testdata_4_decompressed.txt
else
    echo "Decompressor Test #4 FAIL"
fi




# TEST #5
# Tests that near matches should work
../../src/decompressor.exe testdata_5_dict.txt testdata_5_output_golden.hex 4 > testdata_5_decompressed.txt

if diff -qw testdata_5_input.txt testdata_5_decompressed.txt > /dev/null; then
    echo "Decompressor Test #5 PASS"
	rm testdata_5_decompressed.txt
else
    echo "Decompressor Test #5 FAIL"
fi
