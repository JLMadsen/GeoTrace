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
    # todo: download to ./release and copy to OS builds
    # Guide: https://slproweb.com/download/Win64OpenSSL_Light-1_1_1k.exe
    echo "Fetch libssl"
    curl https://denlurevind.com/content/libssl-1_1-x64.dll -O      > /dev/null 2>&1
    echo "Fetch libcrypto"
    curl https://denlurevind.com/content/libcrypto-1_1-x64.dll -O   > /dev/null 2>&1
}

echo "Building Qt project - ${APP}"

if ! hash qmake &> /dev/null
then
    echo "qmake could not be found"
    exit
fi

if ! hash mingw32-make &> /dev/null
then
    echo "mingw32-make could not be found"
    exit
fi

rm -rf ./release

echo "Run qmake"
qmake                           > /dev/null 2>&1
echo "Run mingw32-make"
mingw32-make                    > /dev/null 2>&1

cd ./release

sleep .5

echo "Delete build files"
find . -name "*.o" -type f -delete
find . -name "*.h" -type f -delete
find . -name "*.cpp" -type f -delete

if ! hash windeployqt &> /dev/null
then
    echo "Install windeployqt, and/or add it to path, to build for Windows"
else
    mkdir Windows
    cp "${APP}.exe" "Windows/${APP}.exe"

    cd ./Windows

    echo "Run windeployqt"
    windeployqt "${APP}.exe"            > /dev/null 2>&1
    get_ssl

    echo "Zip Windows"
    tar -cvf "${APP}_Windows.zip" *     > /dev/null 2>&1
    mv "${APP}_Windows.zip" ../.
    cd ..
fi

if ! hash linuxdeployqt &> /dev/null
then
    echo "Install linuxdeployqt, and/or add it to path, to build for Linux"
else
    mkdir Linux
    cp "${APP}.exe" "Linux/${APP}.exe"

    cd ./Linux

    echo "Run linuxdeployqt"
    linuxdeployqt "${APP}.exe"          > /dev/null 2>&1
    get_ssl

    echo "Zip Linux"
    tar -cvf "${APP}_Linux.zip" *       > /dev/null 2>&1
    mv "${APP}_Linux.zip" ../.
    cd ..
fi

rm "${APP}.exe"

echo "Build complete"
# sleep to show echos
sleep 2
