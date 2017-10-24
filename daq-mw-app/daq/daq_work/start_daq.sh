#!/bin/bash
./kill_All.sh

sleep 1
./show_logs.sh
echo "USE Ctrl+C to stop"
sleep 1
#run.py -cl config_all.xml 
xterm -T Operator  -geom 95x15+0+400  -e 'run.py -cl config_all.xml' &
