#!/bin/bash

export LOG_LEVEL=LOG_INF
export PATH="$PATH:$ROOT_DIR/sbin:$ROOT_DIR/bin"

PROGRAM=$(basename $0)

# get parent task name
PNAME=$(cat /proc/$PPID/comm)

# log path
LOG_FILE="/tmp/lkp/log.txt"

# workspace directory
WORKSPACE=$ROOT_DIR/workspace

# LKP tests project
LKP_PRJ=$WORKSPACE/lkp-tests
LKP_URL="https://github.com/intel/lkp-tests.git"

# overlayfs configration
LOWER_DIR=$LKP_PRJ
FSWORKDIR=$WORKSPACE/tmp
LKP_DIR=$ROOT_DIR/lkp-tests
UPPER_DIR=$ROOT_DIR/lkp-mirror
LKP_IN_CONTAINER="/lkp-tests"

# dockerfile path
DOCKER_PATH=$ROOT_DIR/etc/dockerfile

# docker privilege
DOCKER_HAS_ROOT=true
DOCKER_INAME="$(hostname | tr 'A-Z' 'a-z')"

# job path
JOB_WORKDIR=$WORKSPACE/jobs

#src path
SRC_PATH=$ROOT_DIR/src
