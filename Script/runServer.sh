#!/bin/bash

filePath=$1
outPath=$2

# echo $filePath
cd ../Source/Version\ 3/

# OIFS="$IFS"
IFS=$'\n'
max=0
# for process 128 and 64 its getting wrong num of processes
for f in "$filePath"*
do
    num=$(grep -oh '^[0-9]*'  $f | head -1)
    # echo $num
    if (($num > $max)); then
        max=$num
    fi

    # while IFS= read -rn1 a; 
    # do printf %s "${a:-$'\r'}"; 
    # done

    # while IFS= read -n1 char
    # do  
    #     # echo "$char"    
    #     if [ "$char" = "\r" ]
    #     then
    #         echo "end of line shit found!"
    #     fi  
    # done < "$f"

done
max=$((max+1))
numProcesses=$max
echo $numProcesses

./main $numProcesses $filePath $outPath
