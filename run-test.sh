#!/bin/bash

#TODO make the compression ratio calculator into a separate function.
#TODO add a step to actually compile lzcompress
function run_test {
     gzip -k $1 -c > $1.gz
    ../lzcompress $1 $1.lz
    ../lzcompress -u $1.lz $1.out
    #gunzip -k  $1.gz -c > $1.out.2
    echo "     compression ratio lzcompress: $(echo "scale=2; $(stat -c%s "$1.lz") * 100 / $(stat -c%s "$1")" | bc)%"
    echo "     compression ratio gzip: $(echo "scale=2; $(stat -c%s "$1.gz") * 100 / $(stat -c%s "$1")" | bc)%"
    if cmp -s  $1  $1.out; then
	echo "     the files are the same."
    else
	echo "     the files are different."
    fi
}

cd "$(dirname "$0")"
cd tests/
pwd

echo "--- text file with only As ---"
run_test "test-onlyA.txt"
echo "--- text file with only capital letters ---"
run_test "test-AtoZ.txt"
echo "--- text file with normal text ---"
run_test "test-normal.txt"
echo "--- image file ---"
run_test "test-tux.png"
echo "--- audio file ---"
run_test "test-21.mp3"
echo "--- video file ---"
run_test "test-cat.mp4"
echo "--- pdf file ---"
run_test "test-lz77paper.pdf"
