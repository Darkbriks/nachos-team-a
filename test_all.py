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

LINE_LENGTH=200
SEPARATOR_CHAR='-'

TIMEOUT=5  # secondes

PROJECT_ROOT= subprocess.check_output("git rev-parse --show-toplevel", shell=True).decode("utf-8")
PROJECT_ROOT = PROJECT_ROOT[:-1]
BUILD_DIR=PROJECT_ROOT + "/code/build"
TEST_DIR=PROJECT_ROOT + "/result_expected"

tmp_null = PROJECT_ROOT.split("/")
HOME_USER = f"/{tmp_null[1]}/{tmp_null[2]}/{tmp_null[3]}/tmp"

def print_line(text : str, color : str, desc : str) -> None:
    name_length = len(text)
    sep_length = (LINE_LENGTH - name_length - 12) // 2
    sep1 = SEPARATOR_CHAR * sep_length
    sep2 = SEPARATOR_CHAR * (LINE_LENGTH - name_length - 12 - sep_length)
    print(f"{color} {sep1}{text}{sep2} {bcolors.ENDC}")
    if desc != "":
        print(f"{bcolors.OKCYAN} Description : {desc} {bcolors.ENDC}")


def exec_nachos(prog : str, tmp_file : str) -> bool:
    # Si le test prend plus de 5 secondes, on le tue
    with open(tmp_file, "w+") as f:
        try:
            s = subprocess.check_output(f"cd {BUILD_DIR} ; timeout {TIMEOUT}s {prog}", shell=True, stderr=subprocess.STDOUT).decode("utf-8")
            f.write(s)
            if "Machine halting!" in s: # En cas d'arrêt brutal de la machine, par exemple un SEGV, la chaine "Machine halting!" n'est pas présente
                return False
        except subprocess.CalledProcessError as e:
            if e.returncode == 124:
                f.write("TIMEOUT\n")
            else:
                f.write(e.output.decode("utf-8"))
        return True

def verify_exec(file_expect : str, tmp_file : str, name_of_test : str, desc : str, has_failed : bool = True) -> int:
    p = subprocess.run(["diff", "-u", file_expect,  tmp_file], capture_output=True)

    # Si les fichiers sont identiques ou qu'on a un random seed et pas d'erreur fatale
    if p.returncode == 0 or (RS != "" and not has_failed):
        print_line(f" Test {name_of_test} réussi ", bcolors.OKBLUE, "")
        return 0
    print_line(f" Test {name_of_test} échoué ", bcolors.FAIL, desc)
    print(p.stdout.decode("utf-8"))
    print_line("", bcolors.FAIL, "")
    return 1


def test(args : list[str]) -> int:
    file_expect = args[0]
    arguments = args[1]
    name_of_test = args[2]
    desc = args[3]
    tmp_file= HOME_USER + "/" + file_expect
    failed = exec_nachos(arguments, tmp_file)
    return verify_exec(TEST_DIR + "/" +  file_expect, tmp_file, name_of_test, desc, failed)

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

    for arg in sys.argv:
        if "--rs" in arg:
            RS = "-rs "+arg.split("=")[1]
            break
    else:
        RS = ""

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
                   "test_one_thread_join_an_other_without_corner_case.txt",
                   "test_multiplethread_use_putString.txt",
                   "test_autoexit.txt",
                   "test_autoexit2.txt"
                   ]

# la ligne de commande pour nachos
    arguments=[f"./nachos-step{CURRENT_STEP} {RS} -x ./putChar",
               f"./nachos-step{CURRENT_STEP} {RS} -x ./putString",
               f"./nachos-step{CURRENT_STEP} {RS} -x ./putStringError",
               f'echo "Bob" | ./nachos-step{CURRENT_STEP} {RS} -x ./getString',
               f'echo "5" | ./nachos-step{CURRENT_STEP} {RS} -x ./getInt',
               f'echo "-5" | ./nachos-step{CURRENT_STEP} {RS} -x ./getInt',
               f'echo "9999999999" | ./nachos-step{CURRENT_STEP} {RS} -x ./getInt',
               f'echo "-9999999999" | ./nachos-step{CURRENT_STEP} {RS} -x ./getInt',
               f'echo "ab1c" | ./nachos-step{CURRENT_STEP} {RS} -x ./getInt',
               f"./nachos-step{CURRENT_STEP} {RS} -x ./getErrno",
               f"./nachos-step{CURRENT_STEP} {RS} -x ./makethreads",
               f"./nachos-step{CURRENT_STEP} {RS} -x ./testJoin",
               f"./nachos-step{CURRENT_STEP} {RS} -x ./multi_thread_putString",
               f"./nachos-step{CURRENT_STEP} {RS} -x ./testAutoExit",
               f"./nachos-step{CURRENT_STEP} {RS} -x ./testAutoExit2"
               ]


#le nom du test a affiché en cas d'échec
    name_of_test=["Test PutChar" ,
                  "Test PutString normal",
                  "Test PutString overflow buffer",
                  "Test GetString avec EOF",
                  "Test GetInt entier positif",
                  "Test GetInt entier négatif",
                  "Test GetInt entier positif overflow",
                  "Test GetInt entier négatif overflow",
                  "Test GetInt valeur non numérique",
                  "Test GetString taille négative",
                  "Test création de plusieurs threads",
                  "Test ThreadJoin classique",
                  "Test PutString concurrent",
                  "Test terminaison automatique des threads 1",
                  "Test terminaison automatique des threads 2"
                ]

    test_desc=[
        "Test du syscall PutChar depuis un programme utilisateur dans un cas normal.",
        "Test du syscall PutString depuis un programme utilisateur dans un cas normal.",
        "Test du syscall PutString depuis un programme utilisateur avec en entrée une chaîne de caractères plus longue que la taille du buffer.",
        "Test du syscall GetString depuis un programme utilisateur dans un cas normal avec EOF.",
        "Test du syscall GetInt depuis un programme utilisateur avec un entier positif.",
        "Test du syscall GetInt depuis un programme utilisateur avec un entier négatif.",
        "Test du syscall GetInt depuis un programme utilisateur avec un entier positif dépassant la valeur maximale.",
        "Test du syscall GetInt depuis un programme utilisateur avec un entier négatif dépassant la valeur minimale.",
        "Test du syscall GetInt depuis un programme utilisateur avec une chaîne de caractères non numérique.",
        "Test du syscall GetString depuis un programme utilisateur avec en paramètre une taille de chaîne négative. Doit échouer, et errno doit être mis à E_INVAL (1).",
        "Test du lancement de plusieurs threads depuis un programme utilisateur, avec plusieurs niveaux de threads.",
        "Test du syscall ThreadJoin dans un cas classique sans erreur depuis un programme utilisateur.",
        "Test de la gestion concurrente des appels PutString depuis plusieurs threads dans un programme utilisateur.",
        "Test de la terminaison automatique des threads (pas d'appel explicite à ThreadExit) depuis un programme utilisateur.",
        "Test de la terminaison automatique des threads (pas d'appel explicite à ThreadExit) depuis un programme utilisateur."
    ]

    total : int = 0

    if "generate-tests" in sys.argv:
        for i in range(len(file_to_check)):
            generate( [TEST_DIR+"/"+file_to_check[i], arguments[i] ])
        exit(1)

    color = bcolors.FAIL
    exit_code = 1
    for i in range(len(file_to_check)):
        total += test([file_to_check[i], arguments[i], name_of_test[i], test_desc[i]])
    if total == 0:
        color = bcolors.OKGREEN
        exit_code = 0
    print_line(f" {total} test(s) échoué(s) sur {len(file_to_check)} test(s) ", color, "")
    exit(exit_code)