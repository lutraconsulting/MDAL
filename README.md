[![Build Status](https://travis-ci.org/lutraconsulting/MDAL.svg?branch=master)](https://travis-ci.org/lutraconsulting/MDAL)

# MDAL
Mesh Data Abstraction Library

see [Unstructured Mesh Layers](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/119#issuecomment-380018557)

# Tests

see .travis.yml

# Development

format code:
```
cd scripts
./mdal_astyle.sh `find .. -name \*.h* -print -o -name \*.c* -print`
```

or use git pre-commit hook
```
cd MDAL
ln -s ./scripts/mdal_astyle.sh .git/hooks/pre-commit
```
