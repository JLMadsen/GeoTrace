#!/bin/sh

echo "Compiling Qt project"
echo "Start qmake"
qmake                                   > /dev/null 2>&1
echo "Start mingw32-make"
mingw32-make                            > /dev/null 2>&1

cd ./release

echo "Delete build files - *.o"
find . -name "*.o" -type f -delete
echo "Delete build files - *.h"
find . -name "*.h" -type f -delete
echo "Delete build files - *.cpp"
find . -name "*.cpp" -type f -delete

echo "Start windeployqt"
windeployqt GeoTrace.exe                > /dev/null 2>&1

echo "Copy libssl"
cp "C:\Users\Jakob\Documents\openSSL_dlls\libssl-1_1-x64.dll" .
echo "Copy libcrypto"
cp "C:\Users\Jakob\Documents\openSSL_dlls\libcrypto-1_1-x64.dll" .

echo "Build complete"
sleep 2