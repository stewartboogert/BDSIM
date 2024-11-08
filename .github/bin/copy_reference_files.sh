#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <srcdir> <destdir> "
    exit 1
fi

srcdir=$1
destdir=$2

echo Copying root files from $srcdir to $destdir

files=`find $srcdir -name "*.root"`
#echo $files

for file in $files
do
  #echo $file
  destfilepath=$destdir${file//"$srcdir"/}
  mkdir -p `dirname $destfilepath`
  echo `basename $file` $destfilepath
  cp $file $destfilepath
done