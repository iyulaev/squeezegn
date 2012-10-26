#!/bin/bash

# TEST #1
# Only exact matches (and single-character substitutions) should be enabled
../../src/compressor.exe testdata_1_dict.txt testdata_1_input.txt testdata_1_output.hex > /dev/null

if diff -q testdata_1_output.hex_000 testdata_1_output_golden.hex > /dev/null; then
    echo "Compressor Test #1 PASS"
	rm testdata_1_output.hex_000
else
    echo "Compressor Test #1 FAIL"
fi



# TEST #2
# Only exact matches (and single-character substitutions) should be enabled
../../src/compressor.exe testdata_2_dict.txt testdata_2_input.txt testdata_2_output.hex > /dev/null

if diff -q testdata_2_output.hex_000 testdata_2_output_golden.hex > /dev/null; then
    echo "Compressor Test #2 PASS"
	rm testdata_2_output.hex_000
else
    echo "Compressor Test #2 FAIL"
fi




# TEST #3
# Tests that near matches should work
../../src/compressor.exe testdata_3_dict.txt testdata_3_input.txt testdata_3_output.hex > /dev/null

if diff -q testdata_3_output.hex_000 testdata_3_output_golden.hex > /dev/null; then
    echo "Compressor Test #3 PASS"
	rm testdata_3_output.hex_000
else
    echo "Compressor Test #3 FAIL"
fi





# TEST #4
# Tests that near matches should work
../../src/compressor.exe testdata_4_dict.txt testdata_4_input.txt testdata_4_output.hex

if diff -q testdata_4_output.hex_000 testdata_4_output_golden.hex > /dev/null; then
    echo "Compressor Test #4 PASS"
	rm testdata_4_output.hex_000
else
    echo "Compressor Test #4 FAIL"
fi



# TEST #4
# Tests that near matches should work
../../src/compressor.exe testdata_5_dict.txt testdata_5_input.txt testdata_5_output.hex > /dev/null

if diff -q testdata_5_output.hex_000 testdata_5_output_golden.hex > /dev/null; then
    echo "Compressor Test #5 PASS"
	rm testdata_5_output.hex_000
else
    echo "Compressor Test #5 FAIL"
fi

echo "Compressor Test DONE"
