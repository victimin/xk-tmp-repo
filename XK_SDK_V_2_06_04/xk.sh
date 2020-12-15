#!/bin/bash
DOWNLOAD_SDK_PATH='/boot/XandarKardian/script/tmp/xksdk'
DOWNLOAD_SDK_CKSUM_PATH='/boot/XandarKardian/script/tmp/xksdk.cksum'
LOG_PATH='/var/log/xk/sys'
LOG_FILE='update_log.xkl'
XKSDK_BIN_PATH='/usr/bin/xksdk'
NOW=$(date +"%y%m%d_%H%M%S")
NOW_P=$(date +"%y-%m-%d %T")

filename=${LOG_PATH}/${LOG_FILE}
if [ -f ${filename} ]
then
    echo ggggggggggggggggggggggg
    echo "[$NOW_P] Open log file!!" >> ${filename}
else
    echo nnnnnnnnnnnnnnnnnnnnnnnn
    sudo touch ${filename}
    echo "[$NOW_P] Created log file!!" >> ${filename}
fi


echo "          Killing xksdk process!!" >> ${filename}
sudo killall -2 xksdk
sudo wget -O $DOWNLOAD_SDK_PATH http://otatest.xandarkardian.com/firmware/xksdk/download
# echo $DOWNLOAD_SDK_PATH
CKSUM1=$(cat $DOWNLOAD_SDK_CKSUM_PATH)
CKSUM1_SIZE=${#CKSUM1}
CKSUM2_RAW=$(md5sum $DOWNLOAD_SDK_PATH)
CKSUM2=${CKSUM2_RAW:0:CKSUM1_SIZE}
echo $CKSUM1
echo $CKSUM2
echo "${CKSUM1}"
echo "${CKSUM2}"
echo "          File checking..." >> ${filename}
echo $NOW
if [ "${CKSUM1}" = "${CKSUM2}" ]; then
    echo "          File check passed!!" >> ${filename}
    sudo cp $DOWNLOAD_SDK_PATH $XKSDK_BIN_PATH
    echo "          Installed bin file!!" >> ${filename}
else
    echo "          File check failed!!" >> ${filename}
fi
sudo reboot
