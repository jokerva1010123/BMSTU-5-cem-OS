#!/bin/bash
gcc main.c -Wvla -Werror -Wpedantic -Wextra -Wvla -ggdb --coverage -c
gcc process.c -Wvla -Werror -Wpedantic -Wextra -Wvla -ggdb --coverage -c
gcc -o app.exe main.c process.o --coverage