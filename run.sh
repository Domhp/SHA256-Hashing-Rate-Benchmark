#!/bin/bash
Compile: gcc -Wall main.c sha256.c -o main -lpthread
./main 3
sensors