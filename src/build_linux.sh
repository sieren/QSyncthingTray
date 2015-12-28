#!/bin/bash

if [ -z "$QTDIR" ];
  then echo "Please set QTDIR to Qt Compiler. E.g.: \"export QTDIR=~/Qt/5.5/gcc_64/bin/\""; 
  exit 1; 
fi

$QTDIR/qmake -config release .
make

