# set terminal png medium size 800,800
#set output 'IQPlot.png'
set terminal pdfcairo size 8in, 8in color enhanced 
set output 'IQPlot.pdf'


set title "IQ Rings for TX signal at 144.310 MHz and RX at 144.295 MHz\nRX at max gain for all stages. TX at -80dBm"
set xlabel "I"
set ylabel "Q"

plot 'RX_IQ_AutoSettings.dat' using 2:3 with points title 'Automatic IQ Calibration', \
'RX_IQ_1r0_0.dat' using 2:3 with points title 'Manual IQ Setting to <1.0,1e-6>'

