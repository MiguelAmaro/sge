@ECHO OFF

ECHO -------
ECHO -------

PUSHD src

SET WILDCRARDS=*.h *.cpp *.inl *.c

ECHO TODOS FOUND:
FINDSTR -s -n -i -l "TODO" %WILDCRARDS%

POPD

echo -------
echo -------
