#!/bin/env python3
import sys
import subprocess
import os
import time

PROJECT_ROOT= subprocess.check_output("git rev-parse --show-toplevel", shell=True).decode("utf-8")
PROJECT_ROOT = PROJECT_ROOT[:-1]
BUILD_DIR=PROJECT_ROOT + "/code/build"
TEST_DIR=PROJECT_ROOT + "/code/test/network/"


def testChainNMachine(n):

    command = f"cd {BUILD_DIR}" 

    for i in range(n):
        prog = f"./nachos-step6 -m {i} -o {i+1}"

        if (i == n-1):
            prog = f"./nachos-step6 -m {i} -o 0"

        command += f"; {prog}  & cd {BUILD_DIR}"
        


    s1 = subprocess.Popen(f"{command}", shell=True, stdout=subprocess.PIPE , stderr=subprocess.STDOUT)
    s1.wait()
    out, err = s1.communicate()

    res = str(out).replace('"',"'").replace("\\n","\n")
    res = res[2:res.find("Machine"):]
    print(res)
    
        

if __name__ == "__main__":
    testChainNMachine(10)