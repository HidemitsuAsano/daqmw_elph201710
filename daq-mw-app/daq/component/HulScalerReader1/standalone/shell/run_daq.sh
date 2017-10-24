#!/bin/sh
ipaddr=192.168.11.22

#rMacro='macro/check_eff_single.C("'$1'")'
rMacro='macro/check_eff_double.C("'$1'")'

./bin/daq $ipaddr $1 $2
./bin/decoder $1
root -l $rMacro
#./bin/lutmaker $1
