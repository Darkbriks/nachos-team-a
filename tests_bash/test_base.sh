#!/bin/bash

PROJECT_ROOT=$(git rev-parse --show-toplevel)

function echo_in_red(){
    echo -e "\e[31m $1\e[0m"
}

function exec_nachos(){
    cd $PROJECT_ROOT/code/build
    $1 > $tmp_file
}

function verify_exec(){
    cd $PROJECT_ROOT/tests_bash

    if [[ $(diff $1 $2) ]]
    then
        echo_in_red "ça casse sur test : $3"
        diff $1 $2
        exit 1
    fi
    exit 0
}

file_to_check=$1
arguments=$2
name_of_test=$3
tmp_file="/tmp/$file_to_check"


exec_nachos "$arguments"
verify_exec $file_to_check $tmp_file "$name_of_test"
