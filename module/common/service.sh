#!/system/bin/sh
MODDIR=${0%/*}
echo "等待手机启动完毕，以确保时间准确。若只看到这一行内容，请立即联系模块作者！" > /data/adb/turbo-charge/log.txt
until [[ "$(getprop service.bootanim.exit)" == "1" ]]; do
    sleep 1
done
echo "手机启动完毕" >> /data/adb/turbo-charge/log.txt
echo "" >> /data/adb/turbo-charge/log.txt
nohup ${MODDIR}/turbo-charge 2>&1 > /dev/null &
sleep 1
first_process=$(ps -eo comm,pid 2>/dev/null | grep "turbo-charge" 2>/dev/null | awk '{print $2}' 2>/dev/null)
nohup ${MODDIR}/turbo-charge 2>&1 >> /data/adb/turbo-charge/log.txt &
sleep 60
[[ -n "${first_process}" ]] && kill ${first_process}
