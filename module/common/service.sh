#!/system/bin/sh
MODDIR=${0%/*}
[[ -f /data/adb/turbo-charge/log.txt ]] && mv -f /data/adb/turbo-charge/log.txt /data/adb/turbo-charge/log.txt.old
echo "等待手机启动完毕，以确保时间准确。若只看到这一行内容，请立即联系模块作者！" > /data/adb/turbo-charge/log.txt
until [[ "$(getprop service.bootanim.exit)" == "1" ]]; do
    sleep 1
done
echo "手机启动完毕" >> /data/adb/turbo-charge/log.txt
echo "" >> /data/adb/turbo-charge/log.txt
[[ -e /data/vendor/thermal ]] && chattr -i $(find /data/vendor/thermal)
rm -rf /data/vendor/thermal
mkdir -p /data/vendor/thermal/config
chattr +i /data/vendor/thermal/config /data/vendor/thermal
nohup ${MODDIR}/turbo-charge > /dev/null 2>&1 &
sleep 1
first_process=$(ps -eo comm,pid | grep "turbo-charge" | awk '{print $2}')
nohup ${MODDIR}/turbo-charge >> /data/adb/turbo-charge/log.txt 2>&1 &
sleep 60
[[ -n "${first_process}" ]] && kill ${first_process}
