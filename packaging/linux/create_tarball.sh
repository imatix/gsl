#!/bin/bash

PACKAGE_TMP="/tmp"
PACKAGE_NAME="imatix-gsl"
PACKAGE_VERSION="4.1.0"

SCRIPTS_DIR=$(dirname $(cd ${0%/*} 2>>/dev/null ; echo `pwd`/${0##*/}))
PROJECT_DIR="$(realpath ${SCRIPTS_DIR}/../../)"
PROJECT_NAME=$(basename $PROJECT_DIR)

rm -rf $PACKAGE_TMP/$PACKAGE_NAME-$PACKAGE_VERSION
cd $PROJECT_DIR/..
cp -r $PROJECT_NAME ${PACKAGE_TMP}/${PACKAGE_NAME}-${PACKAGE_VERSION}
cd $PACKAGE_TMP/${PACKAGE_NAME}-${PACKAGE_VERSION}/
rm -rf .git
cd ../
tar cfz ${PACKAGE_NAME}-${PACKAGE_VERSION}.tar.gz ${PACKAGE_NAME}-${PACKAGE_VERSION}
cd $SCRIPTS_DIR
pwd
echo "File was created at ${PACKAGE_TMP}/${PACKAGE_NAME}-${PACKAGE_VERSION}.tar.gz"
