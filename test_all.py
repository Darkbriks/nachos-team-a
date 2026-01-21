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

TIMEOUT=8  # secondes

PROJECT_ROOT= subprocess.check_output("git rev-parse --show-toplevel", shell=True).decode("utf-8")
PROJECT_ROOT = PROJECT_ROOT[:-1]
BUILD_DIR=PROJECT_ROOT + "/code/build"
TEST_DIR=PROJECT_ROOT + "/result_expected"

tmp_null = PROJECT_ROOT.split("/")
HOME_USER = f"/{tmp_null[1]}/{tmp_null[2]}/{tmp_null[3]}/tmp"

nachos = ""


class Test:
    name : str
    file_expect : str
    description : str
    line_to_execute : str

    def __init__(self,
                         name : str, file_expect :str,
                         description : str,
                         line_to_execute : str):
        self.name = name
        self.file_expect = file_expect
        self.description = description
        self.line_to_execute = line_to_execute



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
    # if "-x" in prog and CURRENT_STEP == 5:
    print(prog)
    #print(f'{nachos} -f -cp {prog.split("-x")[1]} a')
    s = subprocess.check_output(f"cd {BUILD_DIR} ; {nachos} -f -cp {prog.split('-x')[1]} a", shell=True, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL).decode("utf-8")
    with open(tmp_file, "w+") as f:
        try:
            if "-x" in prog:
                prog = prog.split("-x")[0] + " -x a"
            #print(f"on éxècute {prog}")
            s = subprocess.check_output(f"cd {BUILD_DIR} ; timeout {TIMEOUT}s {prog}", shell=True, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL).decode("utf-8")
            f.write(s)
            if "Machine halting!" in s: # En cas d'arrêt brutal de la machine, par exemple un SEGV, la chaine "Machine halting!" n'est pas présente
                return False
        except subprocess.CalledProcessError as e:
            if e.returncode == 124:
                f.write("TIMEOUT\n")
            else:
                f.write(e.output.decode("utf-8"))
        return True

def verify_exec(file_expect : str, tmp_file : str,
                         name_of_test : str, desc : str, has_failed : bool = True) -> int:
    p = subprocess.run(["diff", "-u", "-E", "-I", "Ticks*", "-I", "Disk*", file_expect,  tmp_file], capture_output=True)

    # Si les fichiers sont identiques ou qu'on a un random seed et pas d'erreur fatale
    if p.returncode == 0 or (RS != "" and not has_failed):
        print_line(f" Test {name_of_test} réussi ", bcolors.OKBLUE, "")
        return 0
    print_line(f" Test {name_of_test} échoué ", bcolors.FAIL, desc)
    print(p.stdout.decode("utf-8"))
    print_line("", bcolors.FAIL, "")
    return 1


def test(arg : Test) -> int:
    file_expect = arg.file_expect
    arguments = arg.line_to_execute
    name_of_test = arg.name
    desc = arg.description
    tmp_file= HOME_USER + "/" + file_expect
    failed = exec_nachos(arguments, tmp_file)
    return verify_exec(TEST_DIR + "/" +  file_expect, tmp_file,
                         name_of_test, desc, failed)

def generate(args : list[str]) -> int:
    file_expect = args[0]
    arguments = args[1]
    try:
        os.remove(file_expect)
    except:
        pass
    exec_nachos(arguments, file_expect)

def compile() -> None:
    s = subprocess.check_output(f"make -C {PROJECT_ROOT}/code FORCE_COMPILATION=1 -j 200 &>/dev/null", shell=True).decode("utf-8")
    

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

    all_test : list[Test] = []
    nachos = f"./nachos-{CURRENT_STEP} {RS} "
    all_test.append(Test(
            file_expect="test_halt.txt",
            line_to_execute =f"{nachos} -x ./halt",
            name= "Halt" ,
            description = "Test du syscall Halt depuis un programme utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_io_output.txt",
            line_to_execute =f"{nachos} -x ./io/test_io_output",
            name = "Syscall PutChar, PutString, PutInt",
            description = "Test des syscalls PutChar, PutString et PutInt depuis un programme utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_getChar.txt",
            line_to_execute =f'echo -e "a\n5\n@\n\n" | {nachos} -x ./io/getChar',
            name = "GetChar",
            description = "Test du syscall GetChar depuis un programme utilisateur dans un cas normal."
        )
    )

    all_test.append(Test(
            file_expect = "test_getString.txt",
            line_to_execute =f'echo "Bob\n\nABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUV\n" | {nachos} -x ./io/getString',
            name = "GetString",
            description = "Test du syscall GetString depuis un programme utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_getStringErrors.txt",
            line_to_execute =f'echo -e "azerty\nazerty\nazerty\nazerty\n" | {nachos} -x ./io/getStringErrors',
            name = "GetString avec erreurs",
            description = "Test du syscall GetString depuis un programme utilisateur avec des tailles invalides."
        )
    )

    all_test.append(Test(
            file_expect = "test_getInt.txt",
            line_to_execute =f'echo "42\n-123\n0\n2147483647\n-2147483648\n 42\n9999999999999\n-9999999999999\nabc\n12abc34\n" | {nachos} -x ./io/getInt',
            name = "GetInt",
            description = "Test du syscall GetInt depuis un programme utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_getIntErrors.txt",
            line_to_execute =f'echo "42\n42\n" | {nachos} -x ./io/getIntErrors',
            name = "GetIntErrors",
            description = "Test du syscall GetInt depuis un programme utilisateur avec des entrées invalides."
        )
    )

    all_test.append(Test(
            file_expect = "test_sleep.txt",
            line_to_execute =f'{nachos} -x ./system/sleep',
            name = "Sleep",
            description = "Test du syscall Sleep depuis un programme utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_sleepUntil.txt",
            line_to_execute =f'{nachos} -x ./system/sleepUntil',
            name = "SleepUntil",
            description = "Test du syscall SleepUntil depuis un programme utilisateur."
        )
    )

    all_test.append(Test(
             file_expect = "test_multithreadSleep.txt",
             line_to_execute =f"{nachos} -x ./system/multithreadSleep",
             name = "Test Sleep concurrent",
             description = "Test de la gestion concurrente des appels Sleep depuis plusieurs threads dans un programme utilisateur."
         )
   )

    all_test.append(Test(
            file_expect = "test_time.txt",
            line_to_execute =f'{nachos} -x ./system/time',
            name = "Time",
            description = "Test du syscall Time et GetCurrentTicks depuis un programme utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_sem.txt",
            line_to_execute = f"{nachos} -x ./sync/sem",
            name = "Vérification du comportement des syscall Semaphore",
            description = "Test pour vérifier le comportement des syscall Semaphore (Create, P, V, Destroy) au niveau utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_ThreadSemaphore.txt",
            line_to_execute =f"{nachos} -x ./sync/testThreadSemaphore",
            name = "Test des sémaphores avec plusieurs threads",
            description = "Test pour vérifier le comportement des sémaphores avec plusieurs threads au niveau utilisateur."
        )
    )

    all_test.append(Test(
            file_expect = "test_ThreadSemaphore2.txt",
            line_to_execute = f"{nachos} -x ./sync/testThreadSemaphore2",
            name = "Test Augmentation automatique de la taille de la table des sémaphores",
            description = "Test pour vérifier l'augmentation automatique de la taille de la table des sémaphores lorsque le nombre de sémaphores dépasse la capacité initiale."
        )
    )


    # FLAG
    # all_test.append(Test(
    #                      file_expect = "test_getString_erno_negative_size.txt",
    #                      line_to_execute =f"{nachos} -x ./system/getErrno",
    #                      name = "Test Errno en contexte global",
    #                      description = "Test des fonctions de récupération d'errno dans un contexte a thread unique, sans tls."
    #                      )
    #                )
    #
    # all_test.append(Test(
    #                      file_expect = "test_lot_of_thread_from_different_functions.txt",
    #                      line_to_execute =f"{nachos} -x ./threads/makethreads",
    #                      name = "Test création de plusieurs threads",
    #                      description = "Test du lancement de plusieurs threads depuis un programme utilisateur, avec plusieurs niveaux de threads."
    #                      )
    #                )
    #
    # all_test.append(Test(
    #                      file_expect = "test_one_thread_join_an_other_without_corner_case.txt",
    #                      line_to_execute =f"{nachos} -x ./threads/testJoin",
    #                      name = "Test ThreadJoin classique",
    #                      description = "Test du syscall ThreadJoin dans un cas classique sans erreur depuis un programme utilisateur."
    #                      )
    #                )
    #
    # all_test.append(Test(
    #                      file_expect = "test_multiplethread_use_putString.txt",
    #                      line_to_execute =f"{nachos} -x ./io/multi_thread_putString",
    #                      name = "Test PutString concurrent",
    #                      description = "Test de la gestion concurrente des appels PutString depuis plusieurs threads dans un programme utilisateur."
    #                      )
    #                )
    #
    # all_test.append(Test(
    #                      file_expect = "test_autoexit.txt",
    #                      line_to_execute =f"{nachos} -x ./threads/testAutoExit",
    #                      name = "Test terminaison automatique des threads 1",
    #                      description = "Test de la terminaison automatique des threads (pas d'appel explicite à ThreadExit) depuis un programme utilisateur."
    #                      )
    #                )
    #
    # all_test.append(Test(
    #                      file_expect = "test_autoexit2.txt",
    #                      line_to_execute =f"{nachos} -x ./threads/testAutoExit2",
    #                      name = "Test terminaison automatique des threads 2",
    #                      description = "Test de la terminaison automatique des threads (pas d'appel explicite à ThreadExit) depuis un programme utilisateur."
    #                      )
    #                )
    #
    #
    # all_test.append(Test(
    #                     file_expect = "test_thread_basicThreadCreate.txt",
    #                     line_to_execute = f"{nachos} -x ./threads/basicThreadsCreate",
    #                     name = "Test basicThreadCreate",
    #                     description = "Test pour vérifier la création de threads basique."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_thread_exitRetVal.txt",
    #                     line_to_execute = f"{nachos} -x ./threads/exitRetVal",
    #                     name = "Test exitRetVal",
    #                     description = "Test pour vérifier la valeur de retour des threads à leur sortie."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_thread_joinOnTerminated.txt",
    #                     line_to_execute = f"{nachos} -x ./threads/joinOnTerminated",
    #                     name = "Test joinOnTerminated",
    #                     description = "Test pour vérifier le comportement de join sur un thread déjà terminé."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_thread_threadAttr.txt",
    #                     line_to_execute = f"{nachos} -x ./threads/threadAttr",
    #                     name = "Test threadAttr",
    #                     description = "Test pour vérifier les attributs des threads."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_thread_threadDetach.txt",
    #                     line_to_execute = f"{nachos} -x ./threads/threadDetach",
    #                     name = "Test threadDetach",
    #                     description = "Test pour vérifier le détachement des threads."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_thread_threadJoinErrors.txt",
    #                     line_to_execute = f"{nachos} -x ./threads/threadJoinErrors",
    #                     name = "Test threadJoinErrors",
    #                     description = "Test pour vérifier la gestion des erreurs lors de l'appel à threadJoin."
    #                 )
    #             )
    # all_test.append(Test(
    #                     file_expect ="test_producteurs_consommateurs.txt",
    #                     line_to_execute = f"{nachos} -x ./sync/producteur_consommateur",
    #                     name = "Test Producteur consommateur ",
    #                     description = "Test pour vérifier cohérence dans le cas d'un producteur/consommateur sur une liste partagée"
    #                 )
    #             )
    # all_test.append(Test(
    #                     file_expect ="test_ForkExec.txt",
    #                     line_to_execute = f"{nachos} -x ./testForkExec",
    #                     name = "Test ForkExec avec trois process",
    #                     description = "Lance un premier process qui va en créer deux autres. Les trois doivent finir de s'éxècuter"
    #                 )
    #             )
    # all_test.append(Test(
    #                     file_expect ="test_HierarchiThread.txt",
    #                     line_to_execute = f"{nachos} -x ./threads/firstThreadCanFinishFirst",
    #                     name = "Test main thread call PthreadExit and others threads can finish",
    #                     description = "Lance un thread depuis main puis appelle PthreadExit. Le thread crée par main doit devenir le principal"
    #                 )
    #             )
    # all_test.append(Test(
    #                     file_expect ="test_exitCode.txt",
    #                     line_to_execute = f'echo "exitCode1" | {nachos} -x ./our_shell',
    #                     name = "Test main thread create a process and check his exitCode",
    #                     description = "Lance le shell pour lui faire exexuter un processus puis verifie son exitCode"
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_ExceptionCatch.txt",
    #                     line_to_execute = f'echo -e "./system/testCorruptionCode\n./system/testStackOverflow\n./system/testCorruptionMemoire\nexit" | {nachos} -x ./our_shell',
    #                     name = "Test main thread create a process and the new one try to write on code section, then create a stack overflow and last try to go after the stack in memory",
    #                     description = "Lance le shell pour lui faire exexuter un processus. Celui ci va faire une erreur que le kernel attrape pour renvoyer une exception et prévenir le shell que son processus fils à mal fini, test 3 exception sur la mémoire"
    #                 )
    #             )
    #
    #
    #
    # all_test.append(Test(
    #                     file_expect ="test_malloc.txt",
    #                     line_to_execute = f"{nachos} -x ./memory_allocator/testMalloc",
    #                     name = "Test malloc and free",
    #                     description = "Test pour vérifier l'allocation et la libération de mémoire dynamique via malloc et free."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_realloc.txt",
    #                     line_to_execute = f"{nachos} -x ./memory_allocator/testRealloc",
    #                     name = "Test realloc",
    #                     description = "Test pour vérifier le comportement de la fonction realloc pour la réallocation de mémoire dynamique."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_corruption_fonction.txt",
    #                     line_to_execute = f"{nachos} -x ./memory_allocator/testCorruptionFonction",
    #                     name = "Test détection de corruption mémoire",
    #                     description = "Test pour vérifier la détection de corruption de la fonction de recherche de bloc mémoire."
    #                 )
    #             )
    #
    # all_test.append(Test(
    #                     file_expect ="test_errno_multithread.txt",
    #                     line_to_execute = f"{nachos} -x ./system/errno_multithread",
    #                     name = "Test errno en contexte multithread avec TLS",
    #                     description = "Test pour vérifier le comportement de errno dans un contexte multithread avec Thread Local Storage (TLS)."
    #                 )
    #             )


    total : int = 0

    if "generate-tests" in sys.argv:
        for i in range(len(all_test)):
            generate( [TEST_DIR+"/"+all_test[i].file_expect, all_test[i].line_to_execute ])
        exit(1)

    color = bcolors.FAIL
    exit_code = 1
    for i in range(len(all_test)):
        total += test(all_test[i])
    if total == 0:
        color = bcolors.OKGREEN
        exit_code = 0
    print_line(f" {total} test(s) échoué(s) sur {len(all_test)} test(s) ", color, "")
    exit(exit_code)
