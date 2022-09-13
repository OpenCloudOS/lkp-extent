#bin/bash
set -e

ROOT_DIR=$(pwd)
echo $ROOT_DIR

#make and install muduo
cd $ROOT_DIR/src/muduo
./build.sh
./build.sh install

cp -r $ROOT_DIR/src/build/release-install-cpp11/include/muduo /usr/include
cp $ROOT_DIR/src/build/release-install-cpp11/lib/* /usr/lib

#make lkp-extent
cd $ROOT_DIR/src
make
echo "lkp-extent ready"

#install lkp-extent
mkdir -p /usr/local/bin
ln -sf $(pwd)/bin/lkp-ctl /usr/local/bin/lkp-ctl


