#!/bin/bash

file_to_check="test_putchar_user_mode_result_expected.txt"
tmp_file="/tmp/tmp_for_project"

function echo_in_red (){
    echo -e "\e[31m $1\e[0m"
}

root_dir=$(git rev-parse --show-toplevel)
cd $root_dir
make -C code
cd code/build
./nachos-step2 -x ./putchar > $tmp_file 

cd $root_dir/tests_bash
if [[ $(diff $tmp_file $file_to_check) ]]
then
    echo_in_red "ça casse"
    exit 1
fi

exit 0




