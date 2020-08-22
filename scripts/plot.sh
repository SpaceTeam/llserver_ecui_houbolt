#!/bin/bash

echo "---generating plots...---"
echo $1;
echo $2;

gnuplot -e "data_path='$1/$2'; img_path='$1/thrust.png'" scripts/thrust_v1.8.plt
gnuplot -e "data_path='$1/$2'; img_path='$1/pressure.png'" scripts/pressure_v1.8.plt
gnuplot -e "data_path='$1/$2'; img_path='$1/temp.png'" scripts/temp_v1.8.plt

echo "---generating plots done---"
