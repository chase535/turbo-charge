#!/system/bin/sh
MODDIR=${0%/*}
echo "$(date "+%Y.%m.%d %H:%M:%S")" > /data/adb/turbo-charge/log.txt
nohup $MODDIR/turbo-charge 2>&1 >> /data/adb/turbo-charge/log.txt &
