sudo killall -2 xksdk 
sudo gpio -g mode 21 output
sudo gpio -g write 21 1
sudo gpio -g mode 19 output
sudo gpio -g write 19 1
sleep 1
sudo gpio -g mode 16 output
sudo gpio -g write 16 1
sudo gpio -g mode 6 output
sudo gpio -g write 6 1
sleep 1
sudo gpio -g mode 20 output
sudo gpio -g write 20 1
sudo gpio -g mode 13 output
sudo gpio -g write 13 1
sleep 1
sudo gpio -g mode 21 output
sudo gpio -g write 21 0
sudo gpio -g mode 19 output
sudo gpio -g write 19 0
sleep 1
sudo gpio -g mode 16 output
sudo gpio -g write 16 0
sudo gpio -g mode 6 output
sudo gpio -g write 16 0
sleep 1
sudo gpio -g mode 20 output
sudo gpio -g write 20 0
sudo gpio -g mode 13 output
sudo gpio -g write 13 0
