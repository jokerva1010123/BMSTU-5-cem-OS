#!/bin/bash
gcc main.c -Wvla -Werror -Wpedantic -Wextra -Wvla -c
gcc functions.c -Wvla -Werror -Wpedantic -Wextra -Wvla -c
gcc -o app.exe main.c functions.o