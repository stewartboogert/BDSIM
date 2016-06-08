#!/bin/sh

source ./setup.sh
source ./setup_gcc.sh

DLDURL=https://www.freedesktop.org/software/fontconfig/release/
DLDFILE=fontconfig-2.11.95.tar.gz
PKGNAME=fontconfig
LOG=$BASEDIR/$PKGNAME.log

##################################################
# freetype
##################################################
cd $BASEDIR

echo "Downloading $PKGNAME" | tee $LOG
wget --quiet $DLDURL/$DLDFILE
mv $DLDFILE ./src/

echo "Unpacking $PKGNAME" | tee -a $LOG
rm -rf $BASEDIR/build/$PKGNAME
mkdir $BASEDIR/build/$PKGNAME 
tar zxf $BASEDIR/src/$DLDFILE -C ./build/$PKGNAME --strip-components=1


rm -rf $BASEDIR/build/$PKGNAME-build/
mkdir $BASEDIR/build/$PKGNAME-build
cd $BASEDIR/build/$PKGNAME-build

echo "Configuring $PKGNAME" | tee -a $LOG
../$PKGNAME/configure --prefix=$INSTALLDIR --disable-docs >> $LOG


echo "Building $PKGNAME" | tee -a $LOG
make -j$NCPU >> $LOG

echo "Installing $PKGNAME" | tee -a $LOG
make install >> $LOG
