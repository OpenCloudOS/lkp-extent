#!/bin/bash

source $ROOT_DIR/lib/log.sh

code_init() {
    local dir
    # 在workspace/lkp-tests安装lkp，作为overlay的lower dir(only read， 不影响lkp项目)
    dir=$(dirname $LKP_PRJ)
    [ ! -d $LKP_PRJ ] && {
        mkdir -p $dir
        cd $dir && git clone $LKP_URL $(basename $LKP_PRJ)
    }

    
}

overlay_init(){
    mkdir -p $FSWORKDIR
    mkdir -p $LKP_DIR

    [ ! -e "$UPPER_DIR" ] && {
        mkdir -p $UPPER_DIR
    }
    #将Upperdir(lkp-mirror)与lowerdir(workspace/lkp-tests)合并挂载到lkp-tests
    mount -t overlay overlay -olowerdir=$LOWER_DIR,upperdir=$UPPER_DIR,workdir=$FSWORKDIR $LKP_DIR -o index=off -o metacopy=off
    [ $? -ne 0 ] && {
        lkp_show "overlayfs init failed"
    } || {
        lkp_show "overlayfs is mounted"
    }
    return 0
}

fs_init(){
    local found=0

    mkdir -p $WORKSPACE
    [ ! -d $LKP_PRJ ] && {
        code_init
    }



    for mountpoint in $(cat /proc/mounts | awk '{print $2}');do
        fstype=$(cat /proc/mounts | grep $mountpoint | awk '{print $1}')

        if [ "$fstype" == "overlay" -a "$mountpoint" == "$LKP_DIR" ];then
            found=1
            break
        fi
    done

    # if not found
    [ $found -eq 0 ] && {
        overlay_init
    } || {
        lkp_show "overlayfs is already mounted"
    }
}

fs_deinit(){
    umount $LKP_DIR
    rm -rf $LKP_DIR
    rm -rf $WORKSPACE
}
