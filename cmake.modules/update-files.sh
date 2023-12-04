#!/bin/sh

SRC_FILELIST=$(pwd)/src/files.cmake
TEST_FILELIST=$(pwd)/tests/files.cmake

SCRIPTPATH=`dirname "$(readlink -f "$0")"`

HEADER_FILTER=".*(h|hpp|inl)$"
SOURCE_FILTER=".*(c|cc|cpp)$"

export LC_ALL=C

#========================================
# Generates the files.cmake file in the current folder
# Arguments:
# 1. string - the files.cmake file with path
# 2. string - the extra include files path to add
#========================================
_generate_filelist()
{
    CMAKE_FILELIST=$1
    INCLUDE_PATH=$2

    echo "set(SOURCES" > ${CMAKE_FILELIST}
    find -E * -type f -regex $SOURCE_FILTER|sort >> ${CMAKE_FILELIST}
    find -E * -type f -regex $HEADER_FILTER|sort >> ${CMAKE_FILELIST}
    if [ ! -z "$INCLUDE_PATH" ] ; then
        find -E $INCLUDE_PATH/* -type f -regex $HEADER_FILTER|sort >> ${CMAKE_FILELIST}
    fi
    echo ")" >> ${CMAKE_FILELIST}
}

cd "${SCRIPTPATH}/../src"

echo "Updating filelist file " ${SRC_FILELIST}
_generate_filelist ${SRC_FILELIST} "../include"

echo "Updating filelist file " ${TEST_FILELIST}
cd "${SCRIPTPATH}/../tests"
_generate_filelist ${TEST_FILELIST}

exit 0
