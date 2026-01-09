#!/bin/env python3
import sys
import subprocess
import os
import time

PROJECT_ROOT= subprocess.check_output("git rev-parse --show-toplevel", shell=True).decode("utf-8")
PROJECT_ROOT = PROJECT_ROOT[:-1]
BUILD_DIR=PROJECT_ROOT + "/code/build"
TIMEOUT=1


prog = "./nachos-step6 -m 0 -o 1"
prog2 = "./nachos-step6 -m 1 -o 0"

print(BUILD_DIR)
s1 = subprocess.Popen(f"cd {BUILD_DIR} ; {prog}", shell=True, stderr=subprocess.STDOUT)
s2 = subprocess.Popen(f"cd {BUILD_DIR} ; {prog}", shell=True, stderr=subprocess.STDOUT)

s1.wait()
print(s1)
s2.wait()
print(s2)