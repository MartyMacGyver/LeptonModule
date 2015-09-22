#!/bin/bash
echo "8" > /sys/class/gpio/export
sleep 1
echo "out" > /sys/class/gpio/gpio8/direction
sleep 1
echo "1" > /sys/class/gpio/gpio8/value
./raspberrypi_video
