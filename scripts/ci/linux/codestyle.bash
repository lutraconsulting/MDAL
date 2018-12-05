#!/usr/bin/env bash

cd scripts; ./mdal_astyle.sh `find .. -name \*.h* -print -o -name \*.c* -print`
cd ..