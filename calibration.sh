sudo crontab -l > my_cron_backup.txt
sudo crontab -r
sudo killall -2 xksdk
sudo killall xksdk 
echo "please wait for a minute"
sleep 3
sudo python3 /var/log/xk/py/rcvDebugData.py all off
sleep 5
cd /home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/
sudo make dbgbin

/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 0 95123 75321 74123 96321
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 1 95123 75321 74123 96321
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 2 95123 75321 74123 96321
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 3 95123 75321 74123 96321
sudo killall -2 xksdk
sudo killall xksdk 
sleep 5

/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 0 1010.1010 6 9 255.255 255.255
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 1 1010.1010 6 9 255.255 255.255
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 2 1010.1010 6 9 255.255 255.255
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 3 1010.1010 6 9 255.255 255.255
sleep 2
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 0 -1
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 1 -1
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 2 -1
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 3 -1
sudo killall -2 xksdk
sudo killall xksdk 
sleep 5

/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 0 5028
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 1 5028
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 2 5028
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 3 5028

sleep 6
sudo xksdk &

sudo crontab /home/xk/my_cron_backup.txt
sudo rm /home/xk/my_cron_backup.txt
