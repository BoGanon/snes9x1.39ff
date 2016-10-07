#!/bin/sh
DATE=`date +%Y%m%d` # year month date
svn export pnd pnd_tmp
cp ../snes9x pnd_tmp/
cp ../README_Snes9x4X.txt pnd_tmp/
./pnd_make.sh -p snes9x4p_$DATE.pnd -d pnd_tmp -x pnd_tmp/PXML.xml -i pnd_tmp/icon.png -c
rm -rf pnd_tmp

