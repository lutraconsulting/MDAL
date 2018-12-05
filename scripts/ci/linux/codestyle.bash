#!/usr/bin/env bash

set -e
cd scripts; ./mdal_astyle.sh `find .. -name \*.h* -print -o -name \*.c* -print`
cd ..