#!/bin/sh

# Build Qt application

# Prerequisite:
# - qmake
# - mingw32-make
# - windeployqt
# - linuxdeployqt       https://github.com/probonopd/linuxdeployqt

APP="GeoTrace"

get_ssl()
{
    # Guide: https://slproweb.com/download/Win64OpenSSL_Light-1_1_1k.exe
    echo "Copy libssl"
    curl https://denlurevind.com/content/libssl-1_1-x64.dll -O      > /dev/null 2>&1
    echo "Copy libcrypto"
    curl https://denlurevind.com/content/libcrypto-1_1-x64.dll -O   > /dev/null 2>&1
}

echo "Building Qt project - ${APP}"

# these need to be in path.
echo "Run qmake"
qmake                           > /dev/null 2>&1
echo "Run mingw32-make"
mingw32-make                    > /dev/null 2>&1

cd ./release

mkdir Windows
mkdir Linux

echo "Delete build files - *.o"
find . -name "*.o" -type f -delete
echo "Delete build files - *.h"
find . -name "*.h" -type f -delete
echo "Delete build files - *.cpp"
find . -name "*.cpp" -type f -delete

cp "${APP}.exe" "Windows/${APP}.exe"
cp "${APP}.exe" "Linux/${APP}.exe"

cd ./Windows

echo "Run windeployqt"
windeployqt "${APP}.exe"            > /dev/null 2>&1
get_ssl

echo "Zip Windows"
tar -cvf "${APP}_Windows.zip" *     > /dev/null 2>&1

cd ../Linux

echo "Run linuxdeployqt"
# linuxdeployqt "${APP}.exe"          > /dev/null 2>&1
get_ssl

echo "Zip Linux"
tar -cvf "${APP}_Linux.zip" *       > /dev/null 2>&1

echo "Build complete"
# sleep to show echos
sleep 4
