#!/usr/bin/env bash

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PWD=`pwd`
cd $DIR
./mdal_astyle.bash `find .. -name \*.h* -print -o -name \*.c* -print -o -path ../mdal/3rdparty -prune`
cd $PWD
