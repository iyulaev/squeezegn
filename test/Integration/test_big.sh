#!/bin/bash

# TEST #1
TEST=1
echo "BIGTEST #$TEST building dictionary"
START_NS=$(date +%s%N)
../../src/dictionarybuilder.exe genome6_a.txt dict6.txt 1000
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Dict Builder took $DELTA_MS to run"

echo "BIGTEST #$TEST compressing"
START_NS=$(date +%s%N)
../../src/compressor.exe dict6.txt genome6_a.txt gen6a_comp
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Compressor took $DELTA_MS to run"

echo "BIGTEST #$TEST decompressing"
START_NS=$(date +%s%N)
../../src/decompressor.exe dict6.txt gen6a_comp_000 > gen6a_decomp.txt
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Decompressor took $DELTA_MS to run"

if diff -qw genome6_a.txt gen6a_decomp.txt > /dev/null; then
    echo "Decompressor Test #$TEST PASS"
	#rm dict1.txt
	#rm gen1_comp*
	#rm gen1_decomp.txt
else
    echo "Decompressor Test #$TEST FAIL"
fi


# TEST #1
TEST=2
# echo "BIGTEST #$TEST building dictionary"
# START_NS=$(date +%s%N)
# ../../src/dictionarybuilder.exe genome5.txt dict5.txt 2000
# END_NS=$(date +%s%N)
# DELTA_MS=$((($END_NS - $START_NS) / 1000000))
# echo "Dict Builder took $DELTA_MS to run"

echo "BIGTEST #$TEST compressing"
START_NS=$(date +%s%N)
../../src/compressor.exe dict6.txt genome6_b.txt gen6b_comp
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Compressor took $DELTA_MS to run"

echo "BIGTEST #$TEST decompressing"
START_NS=$(date +%s%N)
../../src/decompressor.exe dict6.txt gen6b_comp_000 > gen6b_decomp.txt
END_NS=$(date +%s%N)
DELTA_MS=$((($END_NS - $START_NS) / 1000000))
echo "Decompressor took $DELTA_MS to run"

if diff -qw genome6_b.txt gen6b_decomp.txt > /dev/null; then
    echo "Decompressor Test #$TEST PASS"
	#rm dict1.txt
	#rm gen1_comp*
	#rm gen1_decomp.txt
else
    echo "Decompressor Test #$TEST FAIL"
fi


