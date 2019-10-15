reset

set output 'fig1.eps' 
set term postscript eps size 3.13,2.34 enhanced color "Helvetica" 12
set encoding utf8

set logscale x 2
set logscale y

set key left box ls 0 lw 1

set format x '2^{%L}'
set format y '10^{%L}'

set title 'Fig. 1. Input file size vs operation time. Naive implementation'

set xlabel 'File size (B)'
set ylabel 'Time (ns)'

set for [i=10:30:2] xtics (0,2**i)

set xrange [9e2 to 1.1e9]
set yrange [1e3 to 2e11]

plot 'profile.out' u 1:2 w p pt 2 t 'File read time', \
     'profile.out' u 1:3 w p pt 3 t 'Print time', \
     'profile.out' u 1:4 w p pt 4 t 'Product time'
