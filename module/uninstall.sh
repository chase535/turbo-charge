#!/system/bin/sh
chattr -i /data/vendor/thermal
chattr -i /data/vendor/thermal/config
rm -rf /data/adb/turbo-charge
