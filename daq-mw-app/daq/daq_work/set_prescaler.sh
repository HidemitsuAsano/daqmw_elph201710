#!/bin/bash

ip=192.168.10.52

addrCh0B0=0xA0000000
addrCh0B1=0xA0100000

addrCh1B0=0xA0010000
addrCh1B1=0xA0110000

addrGate=0xAF000000

# read current value
#readReg $ip 1 $addrCh0B1
#readReg $ip 1 $addrCh0B0

#readReg $ip 1 $addrCh1B1
#readReg $ip 1 $addrCh1B0


# set ch0 prescaler max count = 10000 (0x2710)
# (NOTE: New value is loaded when gate is off)
/home/daq1/programs/DAQ-Middleware-1.4.2/src/lib/SiTCP/C/sitcpbcp/programs/writeReg $ip $addrCh0B0 0x20
/home/daq1/programs/DAQ-Middleware-1.4.2/src/lib/SiTCP/C/sitcpbcp/programs/writeReg $ip $addrCh0B1 0x00


/home/daq1/programs/DAQ-Middleware-1.4.2/src/lib/SiTCP/C/sitcpbcp/programs/writeReg $ip $addrCh1B0 0x20
/home/daq1/programs/DAQ-Middleware-1.4.2/src/lib/SiTCP/C/sitcpbcp/programs/writeReg $ip $addrCh1B1 0x00



# gate off (and reset counter)
/home/daq1/programs/DAQ-Middleware-1.4.2/src/lib/SiTCP/C/sitcpbcp/programs/writeReg $ip $addrGate 0

# gate on
/home/daq1/programs/DAQ-Middleware-1.4.2/src/lib/SiTCP/C/sitcpbcp/programs/writeReg $ip $addrGate 1
