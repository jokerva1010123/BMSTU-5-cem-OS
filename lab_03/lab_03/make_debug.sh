#!/bin/bash
gcc main.c -Wvla -Werror -Wpedantic -Wextra -Wvla -ggdb --coverage -c
gcc functions.c -Wvla -Werror -Wpedantic -Wextra -Wvla -ggdb --coverage -c
gcc -o app.exe main.c functions.o --coverage