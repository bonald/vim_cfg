#!/bin/bash

cd ..
bnsf > make.log 2>&1

grep "unused variable" make.log >> unuse.log

awk -F ":" '{print $1":"$2}' unuse.log > temp.log

for file in `cat temp.log`
do
   f=$(echo $file|awk -F ":" '{print $1}')
   n=$(echo $file|awk -F ":" '{print $2}')
   src=`basename $f`
   f=$(find . -name $src)
   sed -i "$n{s/.*/####&/}" $f
done

svn st|grep M|awk '{print $2}'| xargs sed -i "/^####/d" 
rm *.log
