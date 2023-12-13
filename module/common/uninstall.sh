#!/system/bin/sh
[[ -e /data/vendor/thermal ]] && chattr -i $(find /data/vendor/thermal)
rm -rf /data/vendor/thermal /data/adb/turbo-charge
mkdir -p /data/vendor/thermal/config
