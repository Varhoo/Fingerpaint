#!/bin/bash

cd lib/cvblob/
cmake .
make 
cd ../../
echo "-----------------------"
echo "copy library cvblob ..."
cp lib/cvblob/lib/libcvblob.a lib/
echo "-----------------------"
echo "copy header file ..."
echo "-----------------------"
cp lib/cvblob/cvBlob/*.h lib/h/
make 