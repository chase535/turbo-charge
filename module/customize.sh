SKIPMOUNT=false
PROPFILE=false
POSTFSDATA=false
LATESTARTSERVICE=true

print_modname()
{
ui_print "
 ********************************************************
 
 - 模块: $MODNAME
 - 模块ID: $MODID
 - 模块版本: $MODversion
 - 作者: $MODAUTHOR
 -      模块介绍↓
 - 使用C语言编写，Github Actions进行静态交叉编译(CMake + aarch64-linux-gnu-gcc + musl libc)
 - 删除温控，关闭阶梯式充电
 - 可选添加温控，默认当手机温度高于52℃时最高充电电流限制为2A
 - 可选添加电量控制，默认电量到达95%时断电，小于等于80%时恢复充电
 - 充电时持续修改电池温度，让系统认为电池温度一直是28℃
 - 持续修改充电电流，以达到最快充电速度
 - 为了避免电池过热强制关机，故做如下调整
   · 若断电后电池温度高于55℃，程序仍会强制显示28℃
   · 待电池温度降至55℃以下，显示电池真实温度

 ！！！若手机体感温度过高，请立即拔下充电器并将手机静置在阴凉处！！！

 ********************************************************

 - 可修改/data/adb/turbo-charge/option.txt来更改一些参数，即时生效
 - 当然也可以通过重新刷模块来选择是否添加温控和电量控制
 - 已知很多手机无法正常使用，不要问我xxx手机行不行，具体自测

 ********************************************************
 "
}

volume_keytest()
{
    ui_print "--- 音量键测试 ---"
    ui_print "  请按音量+或-键"
    (/system/bin/getevent -lc 1 2>&1 | /system/bin/grep VOLUME | /system/bin/grep " DOWN" > "$TMPDIR"/events) || return 1
    return 0
}

volume_choose()
{
    while true; do
        /system/bin/getevent -lc 1 2>&1 | /system/bin/grep VOLUME | /system/bin/grep " DOWN" > "$TMPDIR"/events
        if (`cat "$TMPDIR"/events 2>/dev/null | /system/bin/grep VOLUME >/dev/null`); then
            break
        fi
    done
    if (`cat "$TMPDIR"/events 2>/dev/null | /system/bin/grep VOLUMEUP >/dev/null`); then
        return 1
    else
        return 0
    fi
}

run_volume_key_test()
{
    if volume_keytest; then
        KEYTEST=volume_choose
        ui_print "- 音量键测试完成"
    else
        KEYTEST=false
        ui_print " ！错误：没有检测到音量键选择，默认添加温控、不添加电量控制"
    fi
}

run_temp()
{
    ui_print " "
    ui_print "--- 请选择是否添加温控（默认添加） ---"
    ui_print "  音量+键 = 添加温控，当温度高于52℃时限制最高充电电流为2A，低于45℃时恢复"
    ui_print "  音量-键 = 不添加温控，真·极速快充，高温伤手又伤机，谨慎选择！"
    ui_print " "
    if "$KEYTEST"; then
        ui_print "- 不添加温控"
        sed -i 's/TEMP_CTRL=1/TEMP_CTRL=0/g' $TMPDIR/option.txt
    else
        ui_print "- 添加温控"
    fi
}

run_power_ctrl()
{
    ui_print " "
    ui_print "--- 请选择是否添加电量控制（默认不添加） ---"
    ui_print "  音量+键 = 不添加电量控制"
    ui_print "  音量-键 = 添加电量控制，默认为电量到达95%时断电，小于等于80%时恢复充电"
    ui_print " "
    if "$KEYTEST"; then
        ui_print "- 添加电量控制"
        sed -i 's/POWER_CTRL=0/POWER_CTRL=1/g' $TMPDIR/option.txt
    else
        ui_print "- 不添加电量控制"
    fi
}

on_install()
{
    cp -f $TMPDIR/turbo-charge $MODPATH/turbo-charge
    [[ ! -d /data/adb/turbo-charge ]] && mkdir -p /data/adb/turbo-charge
    cp -f $TMPDIR/option.txt /data/adb/turbo-charge/option.txt
    thermals=`ls /system/bin/*thermal* /system/etc/init/*thermal* /system/etc/perf/*thermal* /system/vendor/bin/*thermal* /system/vendor/etc/*thermal* /system/vendor/etc/powerhint* /system/vendor/etc/init/*_thermal* /system/vendor/etc/perf/*thermal* /system/vendor/lib/hw/thermal* /system/vendor/lib64/hw/thermal* 2>/dev/null | grep -v mi_thermald`
    for thermal in $thermals; do
        [[ ! -d $MODPATH/${thermal%/*} ]] && mkdir -p $MODPATH/${thermal%/*}
        touch $MODPATH/$thermal
    done
    [[ -f /data/current ]] && rm -rf /data/current
    chattr -i /data/vendor/thermal
    chattr -i /data/vendor/thermal/config
    rm -rf /data/vendor/thermal/config/*
    chattr +i /data/vendor/thermal/config
    chattr +i /data/vendor/thermal
}

set_permissions()
{
    set_perm_recursive  $MODPATH  0  0  0755  0644
    chmod 0777 $MODPATH/turbo-charge
    chmod 0777 /data/adb/turbo-charge/option.txt
}
