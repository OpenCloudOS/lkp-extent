#/bin/bash
set -e


source $ROOT_DIR/lib/env.sh
source $ROOT_DIR/lib/log.sh

muduo_init(){
    #make muduo
    [ ! -d $SRC_PATH/muduo ] && {
        cd $SRC_PATH && git clone $MUDUO_URL
        cd $SRC_PATH/muduo && ./build.sh -j2 
    }
    #install muduo
    [ ! -d /usr/include/muduo ] && {
        cd $SRC_PATH/muduo && ./build.sh install 
        cd $SRC_PATH/build/release-install-cpp11/include
        cp -r muduo /usr/include
        cd $SRC_PATH/build/release-install-cpp11/lib
        cp * /usr/lib

    } || {
        lkp_show "muduo is already installed"
    }
}