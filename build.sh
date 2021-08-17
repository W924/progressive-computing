#! /bin/bash

path_cur=$(cd `dirname $0`; pwd)
if [ -d $path_cur/bin ]; then
	cd $path_cur/bin
	rm -rf *
	cd ..
else
	mkdir -p $path_cur/bin
fi

if [ -d $path_cur/build ]; then
	cd $path_cur/build
	rm -rf *
	cd ..
else
	mkdir -p $path_cur/build
fi

cd $path_cur/build
cmake ..
make
