#!/bin/sh

FILELIST=$(pwd)/src/files.cmake

SCRIPTPATH=`dirname "$(readlink -f "$0")"`

HEADER_FILTER=".*(h|hpp|inl)$"
SOURCE_FILTER=".*(c|cc|cpp)$"

export LC_ALL=C

cd "${SCRIPTPATH}/../src"

echo "Updating filelist file " ${FILELIST}

echo "set(SOURCES" > ${FILELIST}
find -E * -type f -regex $SOURCE_FILTER|sort >> ${FILELIST}
find -E ../include/* -type f -regex $HEADER_FILTER|sort >> ${FILELIST}
echo ")" >> ${FILELIST}

exit 0
