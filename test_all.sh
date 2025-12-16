#!/bin/bash

PROJECT_ROOT=$(git rev-parse --show-toplevel)
test_directory="$PROJECT_ROOT/tests_bash"
base_test="$test_directory/test_base.sh"

cd $PROJECT_ROOT
total=0

make -C code &>/dev/null

#ajouter aux trois tableaux pour ajouuter de nouveaux tests

# il suffit juste de créer un fihcier correspondant aux résulatts attendus dans tests_bash
declare -a file_to_check=("test_putchar_user_mode_result_expected.txt" "test_putString_user_mode_expect.txt" "test_putStringError_user_mode_expect.txt" "test_getString_expected.txt")

# la ligne de commande pour nachos
declare -a arguments=("./nachos-step2 -x ./putchar" "./nachos-step2 -x ./putString" "./nachos-step2 -x ./putStringError" 'echo "Bob" | ./nachos-step2 -x ./getString')

#le nom du test a affiché en cas d'échec
declare -a name_of_test=("Test putchar en user mode" "Test putString en user mode" "Test putString avec plus de charactére que taille buffer en user mode" "Test getString normal avec EOF  et putString fais min de taille buffer et quantité demandée")


for ((i=0; i<${#arguments[@]}; i++))
do
    echo "on va executer avec ${file_to_check[$i]} ${arguments[$i]} ${name_of_test[$i]}" 
    $base_test "${file_to_check[$i]}" "${arguments[$i]}" "${name_of_test[$i]}" 
    total=$(( $total + $? ))
done

if [ $total -eq 0 ]
then
    echo -e "\e[32m Tout est passé sur tout les test de chaque questions !!! \e[0m"
fi
