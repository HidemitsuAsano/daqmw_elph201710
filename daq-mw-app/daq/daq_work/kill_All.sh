#!/bin/bash

pgrep -fl Comp
killall -KILL -q xterm
killall -KILL -q DaqOperatorComp
killall -KILL -q BestEffortDispatcherComp
killall -KILL -q NIMEASIROCReader1Comp
killall -KILL -q NIMEASIROCReader2Comp
killall -KILL -q Drs4QdcReader1Comp
killall -KILL -q Drs4QdcReader2Comp
killall -KILL -q HulScalerReader1Comp
killall -KILL -q MonitorComp
killall -KILL -q DAQLoggerComp
killall -KILL -q SampleReaderComp
killall -KILL -q SampleMonitorComp
killall -KILL -q SampleLoggerComp
