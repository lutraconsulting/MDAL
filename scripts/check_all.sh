DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PWD=`pwd`
cd $DIR
CI=0 ./mdal_astyle.sh `find .. -name \*.h* -print -o -name \*.c* -print`
cd $PWD
