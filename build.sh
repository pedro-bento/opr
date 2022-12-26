#!/bin/sh

set -xe

mkdir -p ./build/
cc -std=c17 -Wall -Wextra -ggdb -DEBUG -o ./build/main ./src/main.c