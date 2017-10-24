#!/bin/bash

xterm -T NIMEASIROCReader1 -bg lightblue -geom 100x14+640+0 -e tail -F  /tmp/daqmw/log.NIMEASIROCReader1Comp &
xterm -T NIMEASIROCReader2 -bg lightblue -geom 100x14+640+250 -e tail -F  /tmp/daqmw/log.NIMEASIROCReader2Comp &
xterm -T Drs4QdcReader1 -bg lightblue -geom 100x14+640+480 -e tail -F  /tmp/daqmw/log.Drs4QdcReader1Comp &
xterm -T Drs4QdcReader2 -bg lightblue -geom 100x14+640+710 -e tail -F  /tmp/daqmw/log.Drs4QdcReader2Comp &
xterm -T HulScalerReader1 -bg lightblue -geom 100x14+1280+0 -e tail -F  /tmp/daqmw/log.HulScalerReader1Comp &
xterm -T BestEffortDispatcher -bg pink    -geom 100x14+1280+250 -e tail -F  /tmp/daqmw/log.BestEffortDispatcherComp &
xterm -T Monitor -bg lightgreen -geom 100x14+1280+480 -e tail -F /tmp/daqmw/log.MonitorComp &
xterm -T DAQLogger -bg yellow -geom 100x14+1280+710 -e tail -F /tmp/daqmw/log.DAQLoggerComp &
