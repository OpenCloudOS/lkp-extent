#!/bin/bash

LKP_ELF="lkp"
source $ROOT_DIR/lib/env.sh
source $ROOT_DIR/lib/log.sh

lkp_install () {
    do_make(){
        cd $LKP_DIR
        make && make install
    }

    which $LKP_ELF >/dev/null 2>&1
    [ $? -ne 0 ] && do_make
    $LKP_ELF install $@
}

lkp_run_testcase (){
    local job
    local jobs="$1"
    local basename="$(basename $1)"

    # split jobs
    $LKP_ELF split-job "${jobs}.yaml"
    [ $? -ne 0 ] && {
        lkp_show "No such job: $jobs"
        lkp_log2f LOG_ERR "No such job: $jobs\n"
        return
    }

    # install job
    for job in $(ls ${basename}*);do
        lkp_install $job
    done

    # run job
    for job in $(ls ${basename}*);do
        $LKP_ELF run $job
    done

    # add to results path for use
    
}

# - $1: lkp jobs directory
# - $2: jobs work directory
lkp_run_testcases (){
    local job
    local jobsdir="$1"
    local workdir="$2"

    [ ! -d "$workdir" ] && {
        mkdir -p $workdir
    }

    shift 2
    cd $workdir  # enter work directory
    while [ -n "$1" ]; do
        job="$jobsdir/$1"
        lkp_run_testcase "$job"
        shift 1
    done
    lkp_log2f LOG_INF "lkp run-jobs [$@]"
}
