#!/bin/bash

# files to check
if [ $# -eq 0 ]; then
    # No arguments provided
    FILES=`git diff --name-only --diff-filter=ACMR`
else
    FILES="$@"
fi
echo "running mdal_astyle for $FILES"

# see https://github.com/qgis/QGIS/blob/master/scripts/astyle.options
OPTIONS=$(cat <<-END
--preserve-date
--indent-preprocessor
--convert-tabs
--indent=spaces=2
--indent-classes
--indent-labels
--indent-namespaces
--indent-switches
--max-instatement-indent=40
--min-conditional-indent=-1
--suffix=none
--break-after-logical
--style=allman
--align-pointer=name
--align-reference=name
--keep-one-line-statements
--keep-one-line-blocks
--pad-paren-in
--pad-oper
--unpad-paren
--pad-header
END
)

RETURN=0
ASTYLE=$(which astyle)
if [ $? -ne 0 ]; then
	echo "[!] astyle not installed." >&2
	exit 1
fi

for FILE in $FILES; do
    if [[ $FILE =~ \.(c|cpp|h|hpp)$ ]]; then
        $ASTYLE $OPTIONS < $FILE > $FILE.astyle 
        cmp -s $FILE $FILE.astyle
        if [ $? -ne 0 ]; then
            echo "Changed $FILE" >&2
	    if [ $TRAVIS -eq 1 ]; then
		diff $FILE.astyle $FILE >&2
            fi
	    mv $FILE.astyle $FILE  
            RETURN=1
        else
            rm $FILE.astyle
            echo "Unchanged $FILE" >&2
        fi
    else
       echo "Skipping $FILE" >&2
    fi
done

exit $RETURN
