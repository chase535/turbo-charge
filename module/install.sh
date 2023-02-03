SKIPMOUNT=false
PROPFILE=false
POSTFSDATA=false
LATESTARTSERVICE=true

print_modname()
{
    source ${TMPDIR}/module.prop
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    ui_print " - 模块: ${name}"
    ui_print " - 模块版本: ${version}"
    ui_print " - 作者: ${author}"
    ui_print " -      ↓模块介绍↓"
    ui_print " - 删除温控，充电时持续修改电池温度，让系统认为电池温度一直是28℃"
    ui_print " - 持续修改充电电流，以达到最快充电速度"
    ui_print " - 可选添加温控，若添加，默认当手机温度高于52℃时最高充电电流限制为2A"
    ui_print " - 可选添加电量控制，若添加，默认电量到达95%时断电，小于等于80%时恢复充电"
    ui_print " - 可选是否关闭阶梯式充电，若选是，默认电量大于15%时关闭阶梯式充电，电量小于等于15%时开启阶梯式充电"
    ui_print " - 为了避免电池过热强制关机，故有如下限制"
    ui_print "   · 若不充电时电池温度高于55℃，程序仍会强制显示28℃"
    ui_print "   · 待电池温度降至55℃以下，显示真实温度"
    ui_print " "
    ui_print " ！！！若手机体感温度过高，请立即拔下充电器并将手机静置在阴凉处！！！"
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    ui_print " - 可修改/data/adb/turbo-charge/option.txt来更改一些参数，即时生效"
    ui_print " - 当然也可以通过重新刷模块来选择是否添加温控和电量控制"
    ui_print " "
    ui_print " - log文件为同目录的log.txt，包含配置更改、充电器拔插、撞温度墙等信息"
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    ui_print " ！！！卸载模块请务必在Magisk中卸载！！！"
    ui_print " ！！！或手动执行模块目录下的uninstall.sh文件后再删除模块目录！！！"
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
}

check_file()
{
    ui_print " "
    ui_print "--- 检查所需文件是否存在 ---"
    temp_file=$(ls /sys/class/power_supply/*/temp 2>/dev/null)
    current_max_file=$(ls /sys/class/power_supply/*/constant_charge_current_max /sys/class/power_supply/*/fast_charge_current /sys/class/power_supply/*/thermal_input_current 2>/dev/null)
    for i in $(ls /sys/class/thermal | grep "thermal_zone"); do
        if [[ -f "/sys/class/thermal/${i}/type" ]]; then
            temp_sensor=$(cat /sys/class/thermal/${i}/type 2>/dev/null)
            for j in lcd_therm quiet_therm modem_therm wifi_therm mtktsbtsnrpa mtktsbtsmdpa mtktsAP modem-0-usr modem-1-usr modem1_wifi conn_therm ddr-usr cwlan-usr; do
                [[ "${temp_sensor}" == "${j}" ]] && have_temp_sensor=1 && break
            done
        fi
        [[ -n "${have_temp_sensor}" ]] && break
    done
    if [[ ! -f "/sys/class/power_supply/battery/status" ]]; then
        no_battery_status=1
    fi
    if [[ ! -f "/sys/class/power_supply/battery/capacity" ]]; then
        no_battery_capacity=1
    fi
    if [[ ! -f "/sys/class/power_supply/battery/charging_enabled" && ! -f "/sys/class/power_supply/battery/battery_charging_enabled" && ! -f "/sys/class/power_supply/battery/input_suspend" && ! -f "/sys/class/qcom-battery/restricted_charging" ]]; then
        no_suspend_file=1
    fi
    if [[ -n "${no_battery_status}" || -n "${no_battery_capacity}" || -n "${no_suspend_file}" ]]; then
        ui_print " ！由于找不到有关文件，电量控制功能失效，详情请看程序运行时的log文件！"
        no_power_control=1
    fi
    if [[ ! -f "/sys/class/power_supply/battery/step_charging_enabled" ]]; then
        ui_print " ！由于找不到有关文件，阶梯充电控制的所有功能失效，详情请看程序运行时的log文件！"
        no_step_charging=1
    elif [[ -n "${no_battery_capacity}" ]]; then
        ui_print " ！由于找不到部分有关文件，阶梯式充电控制的部分功能失效，详情请看程序运行时的log文件！"
    fi
    if [[ -z "${current_max_file}" ]]; then
        ui_print " ！由于找不到有关文件，有关电流的所有功能失效，详情请看程序运行时的log文件！"
        no_current_change=1
    fi
    if [[ -z "${temp_file}" || -n "${no_battery_status}" ]]; then
        ui_print " ！由于找不到有关文件，充电时强制显示28℃功能失效，详情请看程序运行时的log文件！"
        no_force_temp=1
    fi
    if [[ -z "${no_force_temp}" || -z "${no_current_change}" ]]; then
        if [[ -z "${have_temp_sensor}" ]]; then
            if [[ -n "${no_force_temp}" ]]; then
                ui_print " ！由于找不到有关温度传感器，温度控制功能失效，详情请看程序运行时的log文件！"
            else
                ui_print " ！由于找不到有关温度传感器，温度控制及充电时强制显示28℃功能失效，详情请看程序运行时的log文件！"
                no_force_temp=1
            fi
        fi
        [[ -n "${no_step_charging}" && -n "${no_power_control}" && -n "${no_force_temp}" && -n "${no_current_change}" && -z "${have_temp_sensor}" ]] && force_exit=1
    else
        [[ -n "${no_step_charging}" && -n "${no_power_control}" && -n "${no_force_temp}" && -n "${no_current_change}" ]] && force_exit=1
    fi
    if [[ -n "${force_exit}" ]]; then
        ui_print " ！所有的所需文件均不存在，完全不适配此手机，安装失败！"
        ui_print " "
        rm -rf ${MODPATH}
        exit 1
    fi
    ui_print "- 检查完毕，开始安装"
}

volume_keytest()
{
    ui_print "--- 音量键测试 ---"
    ui_print "  请按音量+或-键"
    (getevent -lc 1 2>&1 | grep VOLUME | grep " DOWN" > ${TMPDIR}/events) || return 1
    return 0
}

volume_choose()
{
    while true; do
        getevent -lc 1 2>&1 | grep VOLUME | grep " DOWN" > ${TMPDIR}/events
        if (`cat ${TMPDIR}/events 2>/dev/null | grep VOLUME >/dev/null`); then
            break
        fi
    done
    if (`cat ${TMPDIR}/events 2>/dev/null | grep VOLUMEUP >/dev/null`); then
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
        ui_print " ！错误：没有检测到音量键选择，默认不关闭阶梯式充电、添加温控、不添加电量控制"
    fi
}

run_temp()
{
    ui_print " "
    ui_print "--- 请选择是否添加温控（默认添加） ---"
    ui_print "  音量+键 = 添加温控，当温度高于52℃时限制最高充电电流为2A，低于45℃时恢复"
    ui_print "  音量-键 = 不添加温控，真·极速快充，高温伤手又伤机，谨慎选择！"
    ui_print " "
    if ${KEYTEST}; then
        ui_print "- 不添加温控"
        sed -i 's/TEMP_CTRL=1/TEMP_CTRL=0/g' ${TMPDIR}/option.txt
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
    if ${KEYTEST}; then
        ui_print "- 添加电量控制"
        sed -i 's/POWER_CTRL=0/POWER_CTRL=1/g' ${TMPDIR}/option.txt
    else
        ui_print "- 不添加电量控制"
    fi
}

run_step_charge()
{
    ui_print " "
    ui_print "--- 请选择是否关闭阶梯式充电（默认不关闭） ---"
    ui_print "  音量+键 = 不关闭阶梯式充电"
    ui_print "  音量-键 = 关闭阶梯式充电"
    ui_print " "
    if ${KEYTEST}; then
        ui_print "- 关闭阶梯式充电"
        ui_print "- 电池健康状态不好的手机关闭阶梯式充电后低电量充电时会疯狂断充"
        ui_print "- 故在配置文件添加了关闭阶梯式充电的电量阈值，经测试此阈值为15%"
        ui_print "- 且在阶梯充电状态改变时也会造成1-2次的断充"
        sed -i 's/STEP_CHARGING_DISABLED=0/STEP_CHARGING_DISABLED=1/g' ${TMPDIR}/option.txt
    else
        ui_print "- 不关闭阶梯式充电"
    fi
}

on_install()
{
    run_volume_key_test
    run_step_charge
    run_temp
    run_power_ctrl
    ui_print " "
    cp -f ${TMPDIR}/turbo-charge ${MODPATH}
    [[ ! -d /data/adb/turbo-charge ]] && mkdir -p /data/adb/turbo-charge
    cp -f ${TMPDIR}/option.txt /data/adb/turbo-charge
    all_thermal=$(ls /system/bin/*thermal* /system/etc/init/*thermal* /system/etc/perf/*thermal* /system/vendor/bin/*thermal* /system/vendor/etc/*thermal* /system/vendor/etc/powerhint* /system/vendor/etc/init/*thermal* /system/vendor/etc/perf/*thermal* /system/vendor/lib/hw/thermal* /system/vendor/lib64/hw/thermal* 2>/dev/null)
    for thermal in ${all_thermal}; do
        [[ ! -d ${MODPATH}${thermal%/*} ]] && mkdir -p ${MODPATH}${thermal%/*}
        touch ${MODPATH}${thermal}
    done
    chattr -i /data/vendor/thermal
    chattr -i /data/vendor/thermal/config
    rm -rf /data/vendor/thermal/*
    mkdir -p /data/vendor/thermal/config
    chattr +i /data/vendor/thermal/config
    chattr +i /data/vendor/thermal
}

set_permissions()
{
    set_perm_recursive  ${MODPATH}  0  0  0755  0644
    chmod 0777 ${MODPATH}/turbo-charge
    chmod 0777 /data/adb/turbo-charge/option.txt
}

check_file
