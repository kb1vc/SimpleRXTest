#!/bin/bash

mkdir -p build
cd build
cmake ../
make
echo -e  "\n\n\nSupply a 144.310 MHz carrier to the RX port at about -80dBm\n\n"
echo "Hit return when ready"
read ignore_value

./SimpleRXTest
gnuplot ../plot_rings.cmd

echo "Look in the build directory for IQPlot.pdf"
