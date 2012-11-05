#!/bin/bash

# TEST #1
TEST=1
echo "BIGTEST #$TEST building dictionary"
START_NS=$(date +%s%N)
../../src/dictionarybuilder.exe genome1.txt dict1.txt 100000
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Dict Builder took $DELTA_MS to run"

echo "BIGTEST #$TEST compressing"
START_NS=$(date +%s%N)
../../src/compressor.exe dict1.txt genome1.txt gen1_comp
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Compressor took $DELTA_MS to run"

echo "BIGTEST #$TEST decompressing"
START_NS=$(date +%s%N)
../../src/decompressor.exe dict1.txt gen1_comp_000 > gen1_decomp.txt
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Decompressor took $DELTA_MS to run"

if diff -qw genome1.txt gen1_decomp.txt > /dev/null; then
    echo "Decompressor Test #$TEST PASS"
	#rm dict1.txt
	#rm gen1_comp*
	#rm gen1_decomp.txt
else
    echo "Decompressor Test #$TEST FAIL"
fi


# TEST #1
TEST=2

echo "BIGTEST #$TEST compressing"
START_NS=$(date +%s%N)
../../src/compressor.exe dict1.txt genome2.txt gen2_comp
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Compressor took $DELTA_MS to run"

echo "BIGTEST #$TEST decompressing"
START_NS=$(date +%s%N)
../../src/decompressor.exe dict1.txt gen2_comp_000 > gen2_decomp.txt
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Decompressor took $DELTA_MS to run"

if diff -qw genome2.txt gen2_decomp.txt > /dev/null; then
    echo "Decompressor Test #$TEST PASS"
	#rm dict1.txt
	#rm gen1_comp*
	#rm gen1_decomp.txt
else
    echo "Decompressor Test #$TEST FAIL"
fi


