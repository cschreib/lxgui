#!/bin/bash

set -e

# Define which Visual Studio version and which CPU architecture
VS_VERSION=15 # 2017
ARCH=64

# Automatic setup of architecture variables
if [ ${ARCH} == "64" ]; then XARCH=x64; else XARCH=Win32; fi
if [ ${ARCH} == "64" ]; then XARCH2=x64; else XARCH2=x86; fi

# Setup folders
mkdir -p include
mkdir -p lib
mkdir -p bin

SCRIPT_ROOT_DIR=`pwd`

# libpng
LIBPNG_WEBSITE=http://downloads.sourceforge.net/gnuwin32/
LIBPNG_VERSION=1.2.37
wget "${LIBPNG_WEBSITE}libpng-${LIBPNG_VERSION}-bin.zip"
wget "${LIBPNG_WEBSITE}libpng-${LIBPNG_VERSION}-lib.zip"
unzip -o libpng-${LIBPNG_VERSION}-bin.zip
unzip -o libpng-${LIBPNG_VERSION}-lib.zip
rm -rf include/libpng12

# zlib
ZLIB_VERSION=1.2.3
ZLIB_WEBISTE=https://sourceforge.net/projects/gnuwin32/files/zlib/${ZLIB_VERSION}/
wget "${ZLIB_WEBISTE}zlib-${ZLIB_VERSION}-bin.zip"
wget "${ZLIB_WEBISTE}zlib-${ZLIB_VERSION}-lib.zip"
unzip -o zlib-${ZLIB_VERSION}-bin.zip
unzip -o zlib-${ZLIB_VERSION}-lib.zip

# SFML
SFML_VERSION=2.5.1
SFML_WEBISTE=https://www.sfml-dev.org/files/
SFML_DL_FILE=SFML-${SFML_VERSION}-windows-vc${VS_VERSION}-${ARCH}-bit.zip
wget "${SFML_WEBISTE}${SFML_DL_FILE}"
unzip -o ${SFML_DL_FILE}
SFML_DIR="SFML-${SFML_VERSION}"
mv ${SFML_DIR}/include/* include/
mv ${SFML_DIR}/lib/* lib/
mv ${SFML_DIR}/bin/* bin/
rm ${SFML_DL_FILE}
rm -rf ${SFML_DIR}

# Freetype
git clone https://github.com/ubawurinna/freetype-windows-binaries.git
mv freetype-windows-binaries/include/* include/
mv freetype-windows-binaries/win${ARCH}/*.lib lib/
mv freetype-windows-binaries/win${ARCH}/*.dll bin/
rm -rf freetype-windows-binaries

# GLEW
GLEW_VERSION=2.1.0
GLEW_WEBSITE=https://github.com/nigels-com/glew/releases/download/
GLEW_DL_FILE=glew-${GLEW_VERSION}-win32.zip
wget "${GLEW_WEBSITE}glew-${GLEW_VERSION}/${GLEW_DL_FILE}"
unzip ${GLEW_DL_FILE}
GLEW_DIR=glew-${GLEW_VERSION}
mv ${GLEW_DIR}/include/* include/
mv ${GLEW_DIR}/lib/Release/${XARCH}/* lib/
mv ${GLEW_DIR}/bin/Release/${XARCH}/*.dll bin/
rm -rf ${GLEW_DIR}

# Lua (need to build from source)
LUA_VERSION=5.3.0
LUA_WEBSITE=http://www.lua.org/ftp/
LUA_DL_FILE=lua-${LUA_VERSION}.tar.gz
wget ${LUA_WEBSITE}${LUA_DL_FILE}
tar -xvzf ${LUA_DL_FILE}
LUA_DIR=lua-${LUA_VERSION}
mkdir -p ${LUA_DIR}/build
cp LuaCMakeLists.txt ${LUA_DIR}/CMakeLists.txt
cp WindowsSetup${VS_VERSION}.bat ${LUA_DIR}/build/Setup.bat
cp WindowsBuild${VS_VERSION}.bat ${LUA_DIR}/build/Build.bat
cd ${LUA_DIR}/build
cmd.exe /C "Setup.bat"
cmd.exe /C "Build.bat"
cd ../../
mv ${LUA_DIR}/src/*.h ${LUA_DIR}/src/*.hpp include/
mv ${LUA_DIR}/build/*.lib lib/
mv ${LUA_DIR}/build/*.dll bin/
rm -rf ${LUA_DIR}
