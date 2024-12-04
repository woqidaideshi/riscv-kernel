#!/bin/bash

if [ "x$SCHEDULE" == "x1" ]; then
    LTP_PATH=/opt/ltp
    LOG_DIR="$LTP_PATH/log/$(date +%Y%m%d)"
    if [ ! -d $LOG_DIR ]; then
        mkdir -p $LOG_DIR
    fi
    cd $LTP_PATH
    now=$(date +%Y%m%d%H%M%S)
    logfile=$LOG_DIR/ltp_test-$now.log
    logfile_summary=$LOG_DIR/ltp_test_summary-$now.log
    echo "---- LTP test log file:  $logfile ----"
    echo "---- LTP test summary log file:  $logfile_summary ----"
    ./runltp -p -l $logfile_summary -C $logfile_summary.failed -T $logfile_summary.tconf 2>&1 | tee $logfile  # 默认执行全部，scenario_groups/default中配置调度文件;
else
    echo not scheduled, exit 0.
fi