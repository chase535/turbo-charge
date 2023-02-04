#!/system/bin/sh
MODDIR=${0%/*}
echo "等待手机启动完毕，以确保时间准确" > /data/adb/turbo-charge/log.txt
until [[ "$(getprop service.bootanim.exit)" == "1" ]]; do
	sleep 5
done
echo -e "手机启动完毕\n" >> /data/adb/turbo-charge/log.txt
nohup $MODDIR/turbo-charge 2>&1 >> /data/adb/turbo-charge/log.txt &
