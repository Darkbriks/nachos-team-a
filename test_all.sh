PROJECT_ROOT=$(git rev-parse --show-toplevel)
test_directory="$PROJECT_ROOT/tests_bash"
total=0
for elem in $(ls $test_directory/*.sh)
do
    $elem
    total=$(( $total + $? ))
done

if [ $total -eq 0 ]
then
    echo -e "\e[32m Tout est passé sur tout les test de chaque questions !!! \e[0m"
fi
