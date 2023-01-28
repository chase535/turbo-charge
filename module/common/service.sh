#!/system/bin/sh
MODDIR=${0%/*}
nohup $MODDIR/turbo-charge 2>&1 > /data/adb/turbo-charge/log.txt &
