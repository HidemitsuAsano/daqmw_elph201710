#!/bin/bash

ip=192.168.10.16
#reg=./daq/register/LTC2656-12-K1.8-20170419.txt
#offset=./analyzer/offset_table/run02124

reg=./daq/register/LTC2656-12-K1.8-20170525.txt
#offset=./analyzer/offset_table/run02200
#offset=./analyzer/offset_table/run02537
offset=./analyzer/offset_table/run02564

./daq/bin/init_module $ip $reg
./daq/bin/set_ofslut  $ip $offset

