#!/bin/bash
gcc main.c -Wvla -Werror -Wpedantic -Wextra -Wvla -c
gcc process.c -Wvla -Werror -Wpedantic -Wextra -Wvla -c
gcc -o app.exe main.c process.o