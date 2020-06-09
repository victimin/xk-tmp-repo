sudo killall -2 xksdk
sudo killall xksdk 
sleep 5
cd /home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/
sudo make dbgbin
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 0 5028
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 1 5028
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 2 5028
/home/xk/XK_SDK_V_2_04_05_mythings_Rdr_Debug_ver/tools/s2r 3 5028
sleep 6
sudo xksdk &
