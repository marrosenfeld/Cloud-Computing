make
#I./evaluate_cpu $1 $2
gnuplot <<- EOF
	set xlabel "Threads"
	set ylabel "GFLOPS"
	set term png
	set output "plot_by_threads_flops.png"
	set title "Floating Point Operations Performance"
	plot "<(sed -n -e 1p -e 3p -e 5p  'result.csv')"  using 3:xticlabels(1) with linespoints title "GFLOPS"

	set ylabel "GIOPS"
	set output "plot_by_threads_iops.png"
        set title "Integer Operations Performance"
        plot "<(sed -n -e 2p -e 4p -e 6p  'result.csv')"  using 3:xticlabels(1) with linespoints title "GIOPS"

	set ylabel "GIOPS"
	set xlabel "Second"
        set output "plot_in_time_iops.png"
        set title "Integer Operations Performance"
        plot "<(sed -n '1,60p' 'resultInTime.csv')" using 1:2 with linespoints title "GIOPS"

	set ylabel "GFLOPS"
	set xlabel "Second"
	set output "plot_in_time_flops.png"
	set title "Floating Point Operations Performance"
	plot "<(sed -n '61,120p' 'resultInTime.csv')" using 1:2 with linespoints title "GFLOPS"
	
EOF
