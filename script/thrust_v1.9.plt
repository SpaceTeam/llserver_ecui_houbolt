
# befehl zum Aufrufen in der comandozeile: 
# gnuplot -e "data_path='/home/m/Documents/Datenverarbeitung_fuer_TPH_I_138.066/c++_to_gnuplot/2020_01_23__17_16_32.csv'" -e "img_path='/home/m/Documents/Datenverarbeitung_fuer_TPH_I_138.066/c++_to_gnuplot/daten-Plot.png'" /home/m/Documents/Datenverarbeitung_fuer_TPH_I_138.066/c++_to_gnuplot/gnuplot_v1.8.plt

set term png size 1500,800
set output img_path #mit sudo apt-get install gnuplot-x11 geht png

set grid
set datafile separator ";"


stats data_path every ::2::500 using 18 name 'averageTrust1' nooutput #durchscnitt von wert 2 bis 500 herausfinden von colum 3
stats '' every ::2::500 using 19 name 'averageTrust2' nooutput
stats '' every ::2::500 using 20 name 'averageTrust3' nooutput


plot data_path \
using 1:18 title 'Thrust 1 [N]' smooth frequency with lines lw 3,\
'' using 1:19 title 'Thrust 2 [N]' smooth frequency with lines lw 3,\
'' using 1:20 title 'Thrust 3 [N]' smooth frequency with lines lw 3,\
'' using 1:(($18+$19+$20-averageTrust1_mean-averageTrust2_mean-averageTrust3_mean)*8.252e-5) title 'Thrust in N' smooth frequency with lines lw 3,\
'' using 1:20 title 'Thrust Sum in N' smooth frequency with lines lw 3,\
