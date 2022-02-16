#!/bin/bash

source $ROOT_DIR/lib/env.sh
source $ROOT_DIR/lib/log.sh

vol_map="$LKP_DIR:$LKP_IN_CONTAINER"

# - $1: image name
docker_create_image() {
    local found=0
    local docker_images="$1"

    docker images | grep $docker_images >/dev/null 2>/dev/null
    for image in $(docker images | sed '1d' | awk '{print $1}');do
        if [ "$image" == "$docker_images" ];then
            found=1
        fi
    done

    [ $found -eq 0 ] && {
        docker build -t $docker_images:latest $DOCKER_PATH
        lkp_log2f LOG_INF "docker create image: $docker_images\n"
    } || {
        lkp_log2f LOG_INF "docker image:$docker_images is already created\n"
    }
}

# - $1: docker_images
# - $2: container count
docker_create_container (){
    local container_flag
    local docker_images=$1
    local vm_count=${2:-1}

    container_flag="--privileged=$DOCKER_HAS_ROOT -dit"
    for i in $(seq 1 $vm_count); do
        lkp_log2f LOG_INF "docker $i run $docker_images /usr/bin/bash\n";
        docker run $container_flag -v $vol_map --rm $docker_images /usr/bin/bash;
    done
}

# - $1: testcase path
docker_test_container (){
    local testcase="$1"
    local container_flag

    container_flag="--privileged=$DOCKER_HAS_ROOT -dit"
    for id in `docker ps | sed '1 d' | awk '{print $1 }'`; do
        lkp_log2f LOG_INF "docker $i exec $id '$testcase\n"
        docker exec $container_flag $id bash -c "$testcase"
    done
}
