#!/bin/bash

if [ -z "$1" ]; then
  echo "Need a component as a parameter (i.e. dmlite-0.1.0)"
  exit -1
fi
#if [ -z "$2" ]; then
# echo "Need a target"
# exit -1
#fi

SOURCE=${1%%/*}
TARGET=$2
TARGZ="/usr/src/redhat/SOURCES/${SOURCE}.tar.gz"

if [ -z "$TARGET" ]; then
  pushd $1/dist &> /dev/null
  TARGET=`ls *.spec`
  TARGET=${TARGET%%.*}
  echo "TARGET will be $TARGET"
  popd &> /dev/null
fi


if [ -f "${TARGZ}" ]; then
  echo "Removing old tar.gz"
  rm -f ${TARGZ}
fi

echo "Creating ${TARGZ}"
tar vchzf "${TARGZ}" "${SOURCE}"
if [ $? -ne 0 ]; then
  echo "Failed!"
  exit -1
fi
echo

echo "Extracting to /tmp"
tar xzf "${TARGZ}" -C "/tmp"

echo "Changing owner"
chown root.root -R "/tmp/${SOURCE}"


echo "Executing rpmbuild over /tmp/${SOURCE}/dist/${TARGET}.spec"
rpmbuild -bs "/tmp/${SOURCE}/dist/${TARGET}.spec"

