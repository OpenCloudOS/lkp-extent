#!/bin/bash

if [ $# -ne 3 ]; then
    echo "USAGE: ./run.sh casePath workPath taskPath"
    exit 1
fi

lkp install

casePath=$1
workPath=$2
taskPath=$3

mkdir -p $workPath $taskPath

cd $workPath
lkp split $casePath

for file in *.yaml; do
    if [ -f "$file" ]; then
        lkp install $file
        lkp install $file
        break
    fi
done

for file in *.yaml; do
    if [ -f "$file" ]; then
        lkp run $file
        tar zcf $taskPath${file%.yaml}.tar.gz -C result .
    fi
done

tar zcf ${taskPath}rslt.tar.gz -C $taskPath .

exit 0