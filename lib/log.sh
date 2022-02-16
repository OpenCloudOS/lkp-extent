#!/bin/bash

LOG_ERR=0
LOG_WAR=1
LOG_DBG=2
LOG_INF=3

lkp_show(){
    echo "$@"
}

lkp_print(){
    printf "$@"
}

lkp_log(){
    local level
    local log_level
    local max_level

    level="$1"
    log_level=$(eval echo \$$level)
    max_level=$(eval echo \$$LOG_LEVEL)

    shift 1
    [ $log_level -le $max_level ] && { 
        printf "$(date +"%Y-%m-%d %H:%M.%S"): (${level/LOG_/}) $@"
    }
}

lkp_log2f(){
    [ -f $LOG_FILE ] && {
        lkp_log "$@" >> $LOG_FILE
    }
}

lkp_loginit(){
    mkdir -p $(dirname $LOG_FILE)
    [ ! -f $LOG_FILE ] && {
        touch $LOG_FILE
    }
}
