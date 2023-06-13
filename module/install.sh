SKIPMOUNT=false
PROPFILE=false
POSTFSDATA=false
LATESTARTSERVICE=true

check_file()
{
    ui_print " --- 检查所需文件是否存在 ---"
    temp_file=$(ls /sys/class/power_supply/*/temp 2>/dev/null)
    current_max_file=$(ls /sys/class/power_supply/*/constant_charge_current_max /sys/class/power_supply/*/fast_charge_current /sys/class/power_supply/*/thermal_input_current 2>/dev/null)
    for i in $(ls /sys/class/thermal | grep "thermal_zone"); do
        if [[ -f "/sys/class/thermal/${i}/type" ]]; then
            temp_sensor=$(cat /sys/class/thermal/${i}/type 2>/dev/null)
            for j in lcd_therm quiet_therm conn_therm modem_therm wifi_therm mtktsbtsnrpa mtktsbtsmdpa mtktsAP modem-0-usr modem1_wifi ddr-usr cwlan-usr; do
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
    if [[ -n "${no_battery_status}" ]]; then
        ui_print " ！由于找不到有关文件，“伪”旁路供电功能无法根据手机的充电状态而自动启停，详情请看程序运行时的log文件！"
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
        ui_print " ！由于找不到有关文件，有关电流的所有功能（包括“伪”旁路供电功能）失效，详情请看程序运行时的log文件！"
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
    ui_print " - 检查完毕，开始安装"
}

print_modname()
{
    name=
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    ui_print " - 模块: $(grep '^name=' ${TMPDIR}/module.prop | sed 's/^name=//g')"
    ui_print " - 模块版本: $(grep '^version=' ${TMPDIR}/module.prop | sed 's/^version=//g')"
    ui_print " - 作者: $(grep '^author=' ${TMPDIR}/module.prop | sed 's/^author=//g')"
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    check_file
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    ui_print " - 可修改/data/adb/turbo-charge/option.txt来更改一些参数，即时生效"
    ui_print " - log文件为同目录的log.txt，包含文件检测、配置更改等信息"
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    ui_print " ！！！如果安装模块后卡开机，请删除模块目录下的system文件夹！！！"
    ui_print " "
    ui_print " ！！！卸载模块请务必在Magisk中卸载！！！"
    ui_print " ！！！或手动执行模块目录下的uninstall.sh文件后再删除模块目录！！！"
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    ui_print " -      ↓模块介绍↓"
    ui_print " - 删除温控，充电时持续修改电池温度，让系统认为电池温度一直是28℃"
    ui_print " - 持续修改充电电流，以达到最快充电速度"
    ui_print " - 可选添加温控，若添加，默认当手机温度高于52℃时最高充电电流限制为2A"
    ui_print " - 可选添加电量控制，若添加，默认电量到达95%时断电，小于等于80%时恢复充电"
    ui_print " - 可选是否关闭阶梯式充电，若选是，默认电量大于15%时关闭阶梯式充电，电量小于等于15%时开启阶梯式充电"
    ui_print " - 可选是否启用“伪”旁路充电，但请注意是“伪”，因为并不是真的旁路供电，而是直接将充电电流降低至500mA"
    ui_print " "
    ui_print " ！！！若手机体感温度过高，请立即拔下充电器并将手机静置在阴凉处！！！"
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
}

check_old_option()
{
    for i in $(seq $#); do
        VARIABLE=$(eval echo '${'"${i}"'}')
        VALUE=$(grep "^${VARIABLE}=" /data/adb/turbo-charge/option.txt | sed "s/^${VARIABLE}=//g")
        VALUE_DEFAULT=$(grep "^${VARIABLE}=" ${TMPDIR}/option.txt | sed "s/^${VARIABLE}=//g")
        if [[ -z "${VALUE}" ]]; then
            ui_print "  - ${VARIABLE}不存在，使用默认值${VALUE_DEFAULT}"
        elif [[ -z "$(echo "${VALUE}" | grep '^[[:digit:]]*$')" ]]; then
            ui_print "  - ${VARIABLE}的值为空或非纯数字（正整数），使用默认值${VALUE_DEFAULT}"
        elif [[ "${VALUE}" -gt 2147483647 ]]; then
            ui_print "  - ${VARIABLE}的值大于2147483647，这是不被允许的，使用默认值${VALUE_DEFAULT}"
        elif [[ "${VALUE}" -eq 0 && "${VARIABLE}" == "CYCLE_TIME" ]]; then
            ui_print "  - ${VARIABLE}的值等于0，这是不被允许的，使用默认值${VALUE_DEFAULT}"
        else
            ui_print "  - ${VARIABLE}的值为${VALUE}，将此值写入新配置文件"
            sed -i "s/^${VARIABLE}=.*/${VARIABLE}=${VALUE}/g" ${TMPDIR}/option.txt
        fi
    done
}

on_install()
{
    if [[ -f /data/adb/turbo-charge/option.txt ]]; then
        ui_print " 旧配置文件存在，开始读取旧配置文件的配置"
        check_old_option CYCLE_TIME FORCE_TEMP CURRENT_MAX STEP_CHARGING_DISABLED
        check_old_option TEMP_CTRL POWER_CTRL STEP_CHARGING_DISABLED_THRESHOLD
        check_old_option CHARGE_STOP CHARGE_START TEMP_MAX HIGHEST_TEMP_CURRENT
        check_old_option RECHARGE_TEMP BYPASS_CHARGE
    else
        ui_print " 旧配置文件不存在，使用默认配置"
        ui_print "  - 添加温控，当温度高于52℃时限制最高充电电流为2A，低于45℃时恢复"
        ui_print "  - 不添加电量控制"
        ui_print "  - 不关闭阶梯式充电"
        ui_print "  - 启用强制显示28℃功能"
        ui_print "  - 禁用“伪”旁路供电功能"
    fi
    ui_print " "
    ui_print " ********************************************************"
    ui_print " "
    [[ -f /data/adb/turbo-charge/bypass_charge.txt ]] && BYPASS_APP=$(grep -v -E "#|^$" /data/adb/turbo-charge/bypass_charge.txt)
    if [[ -n "${BYPASS_APP}" ]]; then
        echo " 以下APP包名位于旧的“伪”旁路供电列表中，将会写入到新的“伪”旁路供电列表"
        for s in ${BYPASS_APP}; do
            ui_print "  - ${s}"
            echo "${s}" >> ${TMPDIR}/bypass_charge.txt
        done
        ui_print " "
        ui_print " ********************************************************"
        ui_print " "
    fi
    cp -f ${TMPDIR}/turbo-charge ${MODPATH}
    [[ ! -d /data/adb/turbo-charge ]] && mkdir -p /data/adb/turbo-charge
    cp -f ${TMPDIR}/option.txt ${TMPDIR}/bypass_charge.txt /data/adb/turbo-charge
    for k in /system/bin /system/etc/init /system/etc/perf /system/vendor/bin /system/vendor/etc /system/vendor/etc/init /system/vendor/etc/perf; do
        all_thermal="${all_thermal} $(find ${k} -maxdepth 1 -type f -name '*thermal*' 2>/dev/null)"
    done
    all_thermal="${all_thermal} $(find /system/vendor/etc -maxdepth 1 -type f -name 'powerhint*' 2>/dev/null)"
    all_thermal="${all_thermal} $(find /system/vendor/lib/hw -maxdepth 1 -type f -name 'thermal*' 2>/dev/null)"
    all_thermal="${all_thermal} $(find /system/vendor/lib64/hw -maxdepth 1 -type f -name 'thermal*' 2>/dev/null)"
    for thermal in ${all_thermal}; do
        mkdir -p ${MODPATH}${thermal%/*}
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
    set_perm_recursive  ${MODPATH}  0  0  0755  0777
    set_perm_recursive  /data/adb/turbo-charge  0  0  0755  0777
}
