#!/bin/bash

# TEST #1
TEST=1
echo "TEST #$TEST building dictionary"
../../src/dictionarybuilder.exe gen1_head.txt dict1.txt 20000
echo "TEST #$TEST compressing"
START_NS=$(date +%s%N)
../../src/compressor.exe dict1.txt gen1_head.txt gen1_comp
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Compressor took $DELTA_MS to run"
echo "TEST #$TEST decompressing"
../../src/decompressor.exe dict1.txt gen1_comp_000 > gen1_decomp.txt

if diff -qw gen1_head.txt gen1_decomp.txt > /dev/null; then
    echo "Decompressor Test #$TEST PASS"
	#rm dict1.txt
	#rm gen1_comp*
	#rm gen1_decomp.txt
else
    echo "Decompressor Test #$TEST FAIL"
fi


# TEST #2
TEST=2
echo "TEST #$TEST compressing"
START_NS=$(date +%s%N)
../../src/compressor.exe dict1.txt gen2_head.txt gen2_comp
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Compressor took $DELTA_MS to run"
echo "TEST #$TEST decompressing"
../../src/decompressor.exe dict1.txt gen2_comp_000 > gen2_decomp.txt

if diff -qw gen2_head.txt gen2_decomp.txt > /dev/null; then
    echo "Decompressor Test #$TEST PASS"
	#rm dict1.txt
	#rm gen1_comp*
	#rm gen1_decomp.txt
else
    echo "Decompressor Test #$TEST FAIL"
fi

