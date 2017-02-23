#!/bin/bash

tests=(
#"1.in" "2.in" "3.in" "4.in" "5.in" "6.in" 
"8.in" 
#"7.in"
)
execs=("gpu.x" "twocpu.x" 
# "cpu.x"
)
for t in "${tests[@]}"
do
#    echo "$t start:"
    for e in "${execs[@]}"
    do
        echo "$e < $t"
        ./"$e" < "$t"
        echo " "
        echo "$e < $t"
        ./"$e" < "$t"
        echo " "
        echo "$e < $t"
        ./"$e" < "$t"
        echo " "
        echo "$e < $t"
        ./"$e" < "$t"
        echo " "
        echo "$e < $t"
        ./"$e" < "$t"
        echo " "

    done
done
