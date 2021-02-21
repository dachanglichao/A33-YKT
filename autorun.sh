#!/bin/sh

source script_parser.sh

if [ ! -d /system/vendor/ ]; then
    mkdir -p /system/vendor/
    ln -s /lib/modules/`uname -r`/ /system/vendor/modules
    ln -s /boot/*.hcd /system/vendor/modules/
    ln -s /boot/*.bin /system/vendor/modules/
    ln -s /boot/*.txt /system/vendor/modules/
fi

ROOT_DEVICE="/dev/mmcblk0p7"
for parm in $(cat /proc/cmdline); do
    case $parm in
        root=*)
            ROOT_DEVICE=`echo $parm | awk -F\= '{print $2}'`
            ;;
    esac
done

# install nand driver if we boot from sdmmc
nand_activated=`script_fetch "nand" "activated"`
echo "nand activated #$nand_activated"
if [ $nand_activated -eq 1 ]; then
    case $ROOT_DEVICE in
        /dev/mmc*)
      
        nand_module_path=`script_fetch "nand" "module_path"`
        if [ -n "$nand_module_path" ]; then
            insmod "$nand_module_path"
       fi
            ;;
    esac

fi
# insmod touchscreen driver
tp_module_path=`script_fetch "tp" "module_path"`
if [ -n "$tp_module_path" ]; then
    insmod "$tp_module_path"

    # calibrate touchscreen if need
    tp_type=`script_fetch "tp" "type"`
    if [ $tp_type -eq 0 ]; then
        while true; do
            ts_calibrate
            if [ $? -eq 0 ]; then
                break
            fi
        done
    fi

    #ts_test
else
    echo "NO!!! touchscreen driver to be insmod"
fi

#insmod sw-key driver
key_module_path=`script_fetch "key" "module_path"`
insmod "$key_module_path"

# insmod ir driver
ir_activated=`script_fetch "ir" "activated"`
if [ $ir_activated -eq 1 ]; then
    ir_module_path=`script_fetch "ir" "module_path"`
    if [ -n "$ir_module_path" ]; then
        insmod "$ir_module_path"
    fi
fi

# start camera test firstly
while true; do
    camera_activated=`script_fetch "camera" "activated"`
    echo "camera activated #$camera_activated"
    if [ $camera_activated -eq 1 ]; then
        echo "camera activated"
        module_count=`script_fetch "camera" "module_count"`
        if [ $module_count -gt 0 ]; then
            for i in $(seq $module_count); do
                key_name="module"$i"_path"
                module_path=`script_fetch "camera" "$key_name"`
                if [ -n "$module_path" ]; then
                    insmod "$module_path"
                    if [ $? -ne 0 ]; then
                        echo "insmod $module_path failed"
                        break 2
                    fi
                fi
            done
        fi
    else
        echo "camera not activated"
        break
    fi

    echo "camera module insmod done"
    touch /tmp/camera_insmod_done
done

# fix some driver download firmware from /system/vendor/modules.
# android style
if [ ! -d /system/vendor/ ]; then
    mkdir -p /system/vendor/
    ln -s /lib/modules/3.4.39/ /system/vendor/modules
fi

if [ ! -d /data/misc/dmt/ ]; then
    mkdir -p /data/misc/dmt/
fi

mount /dev/mmcblk0p8 /data

insmod /system/vendor/modules/rtl8152.ko
insmod /system/vendor/modules/mma7660.ko
insmod /system/vendor/modules/bcmdhd.ko

insmod /system/vendor/modules/videobuf2-core.ko
insmod /system/vendor/modules/videobuf2-memops.ko
insmod /system/vendor/modules/videobuf2-vmalloc.ko
insmod /system/vendor/modules/uvcvideo.ko
insmod /opt/work/posApp/devrc522.ko
if [ ! -f /usr/lib/libts-0.0.so.0 ]; then
    ln -s /usr/lib/libts-1.0.so.0 /usr/lib/libts-0.0.so.0
fi



#tslib config
export TSLIB_CALIBFILE=/etc/pointercal
TS_INFO_FILE1=/sys/class/input/event3/device/name
TS_INFO_FILE2=/sys/class/input/event4/device/name
if grep -q ft5x_ts $TS_INFO_FILE1; then
   export TSLIB_TSDEVICE=/dev/input/event3
   export QWS_MOUSE_PROTO="Tslib:/dev/input/event3 MouseMan:/dev/input/mouse0"
   if [ ! -s "$TSLIB_CALIBFILE" ]; then
      rm -f $TSLIB_CALIBFILE
   fi
elif grep -q ft5x_ts $TS_INFO_FILE2; then
   export TSLIB_TSDEVICE=/dev/input/event4
   export QWS_MOUSE_PROTO="Tslib:/dev/input/event4 MouseMan:/dev/input/mouse0"
   if [ ! -s "$TSLIB_CALIBFILE" ]; then
      rm -f $TSLIB_CALIBFILE
   fi
else
   export QWS_MOUSE_PROTO=MouseMan:/dev/input/mouse0 > $TSLIB_CALIBFILE
fi
unset TS_INFO_FILE1
unset TS_INFO_FILE2


if [ -f "$TSLIB_CALIBFILE" ]; then
#mainwindow -qws&


#mainwindow 
/opt/work/posApp/lcpos -qws&
/opt/work/net/lcStart.sh -qws&  
else
ts_calibrate
mainwindow -qws&
fi

/opt/work/posApp/testChangeIp

# run dragonboard core process
# core &
