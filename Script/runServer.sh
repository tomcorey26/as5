#!/bin/bash

filePath=$1
outPath=$2

# echo $filePath

# OIFS="$IFS"
IFS=$'\n'
max=0
# for process 128 and 32 its getting wrong num of processes
for f in "$filePath"*
do
    num=$(grep -oh '^[0-9]*'  $f | head -1)
    if (($num > $max)); then
        max=$num
    fi
done
max=$((max+1))
numProcesses=$max
echo $numProcesses
# ../../data_sets/Data\ Sets/End-of-line-fixed/Data\ Set\ 1/
../Source/Version\ 2/main $numProcesses $filePath $outPath
