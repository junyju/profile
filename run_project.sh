#!/bin/sh
  
: '
Author: Alexander Zsikla and Ann Marie Burke
Date: November 18, 2019
Name: run_project.sh

Description: Compiles program, write UM unit tests, and diffs the output
	     of each test with the reference UM command 
	     to ensure correctness. Moves the outputs of both into separate
	     folders.

'

echo -e "\nUPDATING EXECUTABLES\n"

make all

echo -e "\nWRITING TESTS\n"

./writetests

mv *.um test/

echo -e "\nSTARTING TESTING\n"

input="$1"
while IFS= read -r line; do
	filename=$(echo $line | cut -d '.' -f1)

	if [ -e test/"$filename.0" ]; then
		echo "$line"
		# Command UM
		um "test/$line" < test/$filename.0 > test/ref_$filename.1
		# Our UM
		./um "test/$line" < test/$filename.0 > test/$filename.1
		
		diff test/$filename.1 test/ref_$filename.1
	else
		test_name=$line
		echo "Test: $line"
		# Command UM
		um test/$line > test/ref_$filename.1
		# Our UM
		./um test/$line > test/$filename.1

		diff test/ref_$filename.1 test/$filename.1
	fi

	if [ $? -eq 0 ]; then
		echo -e "Test Passed\n"
	else 
		echo -e "Test Failed\n"
	fi

done < "$input"

rm -rf test/outputs/*
rm -rf test/our_outputs/*

mv test/ref_* test/outputs/
mv test/*.1 test/our_outputs/

echo -e "\nSCRIPT COMPLETE\n"

exit 0