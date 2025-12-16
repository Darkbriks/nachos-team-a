#!/bin/env python3
import sys
import subprocess
import difflib

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
TEST_DIR=PROJECT_ROOT + "/tests_bash"



def exec_nachos(prog : str, tmp_file : str) -> None:
    s = subprocess.check_output(f"cd {BUILD_DIR} ; {prog}", shell=True).decode("utf-8")
    with open(tmp_file, "w+") as f:
        f.write(s)

def verify_exec(file_expect : str, tmp_file : str, name_of_test : str) -> None:
    p = subprocess.run(["diff", file_expect, tmp_file], capture_output=True)
    if p.returncode == 0:
        print(f"{bcolors.OKBLUE} ---------------------------------------------------- Test {name_of_test} réussi------------------------------------------------{bcolors.ENDC}")
        return
    print(f"{bcolors.FAIL} ---------------------------------------------------- Test {name_of_test} échoué ------------------------------------------------{bcolors.ENDC}")
    print(p.stdout.decode("utf-8"))
    print(f"{bcolors.FAIL} ---------------------------------------------------- Test {name_of_test} échoué ------------------------------------------------{bcolors.ENDC}")
    exit(1)


def test(args : list[str]) -> None:
    file_expect = args[1]
    arguments = args[2]
    name_of_test = args[3]
    tmp_file="/tmp/" + file_expect
    exec_nachos(arguments, tmp_file)
    verify_exec(TEST_DIR + "/" +  file_expect, tmp_file, name_of_test)


if __name__ == "__main__":
    test(sys.argv)





