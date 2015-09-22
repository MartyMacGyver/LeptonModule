#!/bin/bash
#MYPORT=8  # GPIO8 (CE0)
MYPORT=7  # GPIO7 (CE1)
echo "Toggling GPIO${MYPORT}"
echo "${MYPORT}" > /sys/class/gpio/export
sleep 1
echo "out" > /sys/class/gpio/gpio${MYPORT}/direction
sleep 1
echo "1" > /sys/class/gpio/gpio${MYPORT}/value
sleep 2
#echo "0" > /sys/class/gpio/gpio${MYPORT}/value
#sleep 1
echo "${MYPORT}" > /sys/class/gpio/unexport
echo "Reloading kernel driver"
rmmod spi-bcm2835
modprobe spi-bcm2835

