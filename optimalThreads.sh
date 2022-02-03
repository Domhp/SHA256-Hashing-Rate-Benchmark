#!/bin/bash
gcc -Wall optimalThreads.c sha256.c -o optThreads -lpthread
./optThreads
sensors