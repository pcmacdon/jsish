#!/bin/sh
rm -rf Configs.new
mkdir Configs.new
VER=`jsish -e 'Info.version(".")'`
for i in `ls Configs`; do
    FN=`basename $i .conf| cut -b6-`
    echo $FN
    ./configure --config=$FN
    sed "s/DEFCONFIG_VER=[0-9.]*\$/DEFCONFIG_VER=$VER/"  make.conf > Configs.new/$i
done
