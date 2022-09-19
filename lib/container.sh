#!/bin/bash

source $ROOT_DIR/lib/docker.sh

do_container_test(){
    local container_type=$1

    shift 1
    #${container_type}_create_image $DOCKER_INAME
    ${container_type}_create_container $DOCKER_INAME $VM_CNT
    ${container_type}_test_container $1 
    tar $ROOT_DIR/results/local/result.tar $ROOT_DIR/lkp-tests/dockerResults/*
}
