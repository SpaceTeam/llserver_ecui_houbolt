
# befehl zum Aufrufen in der comandozeile: 
# gnuplot -e "data_path='/home/m/Documents/Datenverarbeitung_fuer_TPH_I_138.066/c++_to_gnuplot/2020_01_23__17_16_32.csv'" -e "img_path='/home/m/Documents/Datenverarbeitung_fuer_TPH_I_138.066/c++_to_gnuplot/daten-Plot.png'" /home/m/Documents/Datenverarbeitung_fuer_TPH_I_138.066/c++_to_gnuplot/gnuplot_v1.8.plt

set term png size 1500,800
set output img_path #mit sudo apt-get install gnuplot-x11 geht png

set grid
set datafile separator ";"

plot data_path \
using 1:16 title 'Oxidizer Tank Bottom Temp [C]' smooth frequency with lines lw 3,\
'' using 1:13 title 'Oxidizer Tank Middle Bottom Temp [C]' smooth frequency with lines lw 3,\
'' using 1:10 title 'Oxidizer Tank Middle Top Temp [C]' smooth frequency with lines lw 3,\
'' using 1:7 title 'Oxidizer Tank Top Temp [C]' smooth frequency with lines lw 3,\
