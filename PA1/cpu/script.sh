make
./evaluate_cpu
gnuplot <<- EOF
	set xlabel "seconds"
	set ylabel "GFLOPS"
	set term png
	set output "GIOPS.png"
	plot "<(sed -n '1,60p' resultInTime.csv )" with lines
	set output "GFLOPS.png"
	plot "<(sed -n '61,120p' resultInTime.csv )" with lines
EOF
