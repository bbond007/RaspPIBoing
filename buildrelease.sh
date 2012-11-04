#!/bin/sh
astyle *.c
rm -f *.orig
rm -f DEADJOE
rm -f *~
make clean
rm -f soundraw.c
rm -f bin2c
rm -f output.raw
cd ..
rm -f RaspPIBoingSource.ZIP
zip -r RaspPIBoingSource.ZIP RaspPIBoing/*
cd RaspPIBoing
./convertwav16.sh
make all
strip RaspPIBoing.bin
zip RaspPIBoing.bin.ZIP RaspPIBoing.bin
cp RaspPIBoing.bin.ZIP ..
echo DONE!!!
ls -la ../RaspPIBoingSource.ZIP ../RaspPIBoing.bin.ZIP


