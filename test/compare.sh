#!/bin/bash

outfile=""
base_folder="./resources"
average_percent=0
usage_string=\
"Compare hits in lfuda algorithm and hits in belady's algorithm (ideal cache replacement policy)\n\
on tests in \"resources\" folder.\n\n\
usage: ./compare.sh [options] [file]\n\
Options:\n\
\t-o --output <file>\tPlace the output into <file>.\n\
\t-h --hlep         \tDisplay this information."

options=$(getopt -o ho: --long help,output: -- "$@")
if [[ $? -ne 0 ]]; then
    exit 1
fi

eval set -- "${options}"

while [ : ]; do
    case "$1" in
    -h | --help)
        echo -e "$usage_string" 1>&2
        exit 0
        ;;
    -o | --output)
        if [[ -n ${outfile} ]]; then
            echo "output file can't be set more then 1 time"
            exit 1
        fi
        shift
        echo "Output placed into $1"
        exec 1>$1
        ;;
    --)
        shift
        break
        ;;
    esac
    shift
done

echo -e "Start testing...\n" 1>&2

test_n=$(ls ${base_folder} | wc -l)
((test_n = test_n - 2))

for (( i = 1; i < ${test_n}; i++ )); do
    ./../build/test/lfuda/lfudacpp < ${base_folder}/test${i}.dat > ${base_folder}/temp-lfuda.dat
    ./../build/test/belady/belady < ${base_folder}/test${i}.dat > ${base_folder}/temp-belady.dat
    echo -e "\t\t\ttest: $test${i}.dat"
    cache_size=$(awk '{print $1}' resources/test${i}.dat)
    echo -e "\t\t\tcache size =  ${cache_size}"
    size_of_test=$(awk '{print $2}' resources/test${i}.dat)
    echo -e "\t\t\ttest size = ${size_of_test}"
    echo "-----------------------------------------------------------------------"
    echo -e "lfuda\t\t\t\t\t\t\t\tbelady\n"
    diff -y ${base_folder}/temp-lfuda.dat ${base_folder}/temp-belady.dat
    lfuda_res=$(cat ${base_folder}/temp-lfuda.dat)
    belady_res=$(cat ${base_folder}/temp-belady.dat)

    percent=$(echo "scale=2; $lfuda_res / $belady_res" | bc -l)
    average_percent=$(echo "scale=2; $average_percent + $percent" | bc -l)
    echo "======================================================================="
done
average_percent=$(echo "scale=2; $average_percent / $test_n" | bc -l)
echo -e "\nRESULT: The average ratio of hits in lfuda to hits in belady is 0$average_percent\n"


