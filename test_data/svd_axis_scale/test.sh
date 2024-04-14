#!/bin/bash

echo "Removing old test.pdb if present:"
rm test.pdb

echo "Running RoPE GUI instructions"
rope.gui.test instructions.txt

echo "Using CCP4 tool superpose to check alignment:"
alignment=`superpose dark.pdb test.pdb | grep 'quality Q' | cut -c -21 | rev | cut -c -6 | rev`
echo "Acquired alignment: " $alignment

result=`echo $alignment | awk ' { print ($1 > 0.99 ? "true" : "false"); } '`
echo "Success: $result"
echo $result > results.txt
