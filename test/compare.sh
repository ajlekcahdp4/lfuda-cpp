#!/bin/bash

outfile=""
base_folder="./resources"

options=$(getopt -o ho: --long help,output: -- "$@")
if [[ $? -ne 0 ]]; then
    exit 1
fi

eval set -- "${options}"

while [ : ]; do
    case "$1" in
    -h | --help)
        echo "Help message" 1>&2
        ;;
    -o | --output)
        if [[ -n ${outfile} ]]; then
            echo "output file can't be set more then 1 time"
            exit 1
        fi
        shift
        exec 1>$1
        ;;
    --)
        shift
        break
        ;;
    esac
    shift
done

test_n=$(ls ${base_folder} | wc -l)
((test_n = test_n - 2))

echo -e "lfuda\t\t\t\t\t\t\t\tbelady\n"

for (( i = 1; i < ${test_n}; i++ )); do
    ./../build/test/lfuda/lfudacpp < ${base_folder}/test${i}.dat > ${base_folder}/temp-lfuda.dat
    ./../build/test/belady/belady < ${base_folder}/test${i}.dat > ${base_folder}/temp-belady.dat
    diff -y ${base_folder}/temp-lfuda.dat ${base_folder}/temp-belady.dat
    echo "-----------------------------------------------------------------------"
done


