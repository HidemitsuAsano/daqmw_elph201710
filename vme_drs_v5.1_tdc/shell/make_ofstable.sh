#!/bin/sh

if [ $# -lt 2 ]; then
    echo "usage:"
    echo " $0 [IP address] [run number]"
    exit 1
fi

echo "board = $1, run = $2"

board=$1
echo "IP address = $board"

run=`printf %05d $2`
echo "run number = $run"

outdir=offset_table/run$run

if [ -e ${outdir} ]; then
    echo "file ${outdir} already exists."
    exit 1
fi


echo "________________________________________"
echo "zero fill before offset calibration"
echo "________________________________________"
cd daq
echo "./bin/set_ofslut $board ./pede_zero"
./bin/set_ofslut $board ./pede_zero

echo "________________________________________"
echo "start offset calibration run"
echo "________________________________________"
echo "./bin/ofstable $board $run"
./bin/ofstable $board $run

cd ../analyzer
mkdir -p ${outdir}
echo "./bin/ofstable_maker ../daq/data/run${run}.dat.gz ${outdir}/table.root"
./bin/ofstable_maker ../daq/data/run${run}.dat.gz ${outdir}/table.root
mv -f *txt ${outdir}

echo "________________________________________"
echo "send offset data"
echo "________________________________________"
cd ../daq
echo "./bin/set_ofslut $board ${outdir}"
./bin/set_ofslut $board ../analyzer/${outdir}
