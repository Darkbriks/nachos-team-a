#!/bin/env python3
import sys
import subprocess
import os
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

PROJECT_ROOT= subprocess.check_output("git rev-parse --show-toplevel", shell=True).decode("utf-8")
PROJECT_ROOT = PROJECT_ROOT[:-1]
BUILD_DIR=PROJECT_ROOT + "/code/build"
TEST_DIR=PROJECT_ROOT + "/result_expected"

tmp_null = PROJECT_ROOT.split("/")
HOME_USER = f"/{tmp_null[1]}/{tmp_null[2]}/{tmp_null[3]}/tmp"


def exec_nachos(prog : str, tmp_file : str) -> None:
    s = subprocess.check_output(f"cd {BUILD_DIR} ; {prog}", shell=True).decode("utf-8")
    with open(tmp_file, "w+") as f:
        f.write(s)

def verify_exec(file_expect : str, tmp_file : str, name_of_test : str) -> int:
    p = subprocess.run(["diff", file_expect, tmp_file], capture_output=True)
    if p.returncode == 0:
        print(f"{bcolors.OKBLUE} ---------------------------------------------------- Test {name_of_test} réussi------------------------------------------------{bcolors.ENDC}")
        return 0
    print(f"{bcolors.FAIL} ---------------------------------------------------- Test {name_of_test} échoué ------------------------------------------------{bcolors.ENDC}")
    print(p.stdout.decode("utf-8"))
    print(f"{bcolors.FAIL} ---------------------------------------------------- Test {name_of_test} échoué ------------------------------------------------{bcolors.ENDC}")
    return 1


def test(args : list[str]) -> int:
    file_expect = args[0]
    arguments = args[1]
    name_of_test = args[2]
    tmp_file= HOME_USER + "/" + file_expect
    exec_nachos(arguments, tmp_file)
    return verify_exec(TEST_DIR + "/" +  file_expect, tmp_file, name_of_test)

def generate(args : list[str]) -> int:
    file_expect = args[0]
    arguments = args[1]
    os.remove(file_expect)
    exec_nachos(arguments, file_expect)

def compile() -> None:
    s = subprocess.check_output(f"make -C {PROJECT_ROOT}/code &>/dev/null", shell=True).decode("utf-8")
    

def usage():
    print(f"{sys.argv[0]} <the step to execute> <optionnal generate-tests> <optionnal skip-compile>")


if __name__ == "__main__":

    if not ("skip-compile" in sys.argv):
        compile()

    if len(sys.argv) == 1:
        usage()
        exit(1)
    CURRENT_STEP = sys.argv[1]
    file_to_check=["test_putchar_user_mode_result_expected.txt",
                   "test_putString_user_mode_expect.txt",
                   "test_putStringError_user_mode_expect.txt",
                   "test_getString_expected.txt",
                   "test_getInt_positive_integer_expected.txt",
                   "test_getInt_negative_integer_expected.txt",
                   "test_getInt_positive_integer_overflow_expected.txt",
                   "test_getInt_negative_integer_overflow_expected.txt",
                   "test_getInt_erno_not_integer_value_expected.txt",
                   "test_getString_erno_negative_size.txt",
                   "test_lot_of_thread_from_different_functions.txt",
                   "test_one_thread_join_an_other_without_corner_case.txt"
                   ]

# la ligne de commande pour nachos
    arguments=[f"./nachos-step{CURRENT_STEP} -x ./putChar",
               f"./nachos-step{CURRENT_STEP} -x ./putString",
               f"./nachos-step{CURRENT_STEP} -x ./putStringError",
               f'echo "Bob" | ./nachos-step{CURRENT_STEP} -x ./getString',
               f'echo "5" | ./nachos-step{CURRENT_STEP} -x ./getInt',
               f'echo "-5" | ./nachos-step{CURRENT_STEP} -x ./getInt',
               f'echo "9999999999" | ./nachos-step{CURRENT_STEP} -x ./getInt',
               f'echo "-9999999999" | ./nachos-step{CURRENT_STEP} -x ./getInt',
               f'echo "ab1c" | ./nachos-step{CURRENT_STEP} -x ./getInt',
               f"./nachos-step{CURRENT_STEP} -x ./getErrno",
               f"./nachos-step{CURRENT_STEP} -rs 5 -x ./makethreads",
               f"./nachos-step3 -x ./testJoin"
               ]


#le nom du test a affiché en cas d'échec
    name_of_test=["Test putchar en user mode", 
                  "Test putString en user mode",
                  "Test putString avec plus de caractères que taille buffer en user mode",
                  "Test getString normal avec EOF  et putString fais min de taille buffer et quantité demandée",
                  "Test getInt avec un integer positif (5)",
                  "Test getInt avec un integer négatif (-5)",
                  "Test getInt avec un integer positif dépasssant la valeur maximale de l'integer (9999999999)",
                  "Test getInt avec un integer négatif dépasssant la valeur minimale de l'integer (-9999999999)",
                  "Test getInt avec une chaîne de caractères qui n'est pas un nombre",
                  "Test getString avec taille négative, renvoie un -1 donc aller voir E_INVAL (1)",
                  "Test le lancement de plusieurs threads. Certains lancés depuis le main et d'autre depuis d'autres threads",
                  "Test vérification de threadJoin dans le cas classsique sans erreur ( thread existant )"
                ]
    total : int = 0

    if "generate-tests" in sys.argv:
        for i in range(len(file_to_check)):
            generate( [TEST_DIR+"/"+file_to_check[i], arguments[i] ])
        exit(1)

    for i in range(len(file_to_check)):
        total += test( [file_to_check[i], arguments[i], name_of_test[i] ])
    if total == 0:
        print(f"{bcolors.OKCYAN} ---------------------------------------------------- Tout marche ------------------------------------------------{bcolors.ENDC}")






