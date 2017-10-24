#!/bin/bash
echo "This PC (usb-lan) 192.168.10.2  "
echo "This PC (PCI)     192.168.10.16 "
echo "NIMEASIROC1       192.168.10.16 "
echo "NIMEASIROC2       192.168.10.100"
echo "HUL-Scaler        192.168.11.21 "
echo "HUL-Scaler(dummy) 192.168.10.21 "
echo "Drs4Qdc1          192.168.10.50 "
echo "Drs4Qdc2          192.168.10.51 "
echo "prescaler         192.168.10.52 "

echo "  "
echo "  "

echo "connected IP in  192.168.10.{1..254}: "
echo "  "
echo 192.168.10.{1..254} | xargs -P256 -n1 ping -s1 -c1 -W1 | grep --color=auto 'ttl'
echo 192.168.11.{1..254} | xargs -P256 -n1 ping -s1 -c1 -W1 | grep --color=auto 'ttl'

