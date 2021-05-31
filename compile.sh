#!/bin/sh

# Simple shell script for building a dynamic build of a Qt application.
# Should work for most projects.
# If you want to use this remember to change filename on windeployqt and
# supply your own LibSSL and LibCrypto dll.
# LibSSL and LibCrypto are only necessary if the application uses HTTPS requests.
# Not tested this on Linux yet.

echo "Building Qt project"

echo "Run qmake"
qmake                                   > /dev/null 2>&1
echo "Run mingw32-make"
mingw32-make                            > /dev/null 2>&1

cd ./release

echo "Delete build files - *.o"
find . -name "*.o" -type f -delete
echo "Delete build files - *.h"
find . -name "*.h" -type f -delete
echo "Delete build files - *.cpp"
find . -name "*.cpp" -type f -delete

echo "Run windeployqt"
windeployqt GeoTrace.exe                > /dev/null 2>&1

echo "Copy libssl"
cp "C:\Users\Jakob\Documents\openSSL_dlls\libssl-1_1-x64.dll" .
echo "Copy libcrypto"
cp "C:\Users\Jakob\Documents\openSSL_dlls\libcrypto-1_1-x64.dll" .

echo "Build complete"
sleep 2