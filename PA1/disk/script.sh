make
#./evaluate_disk $1 $2
gnuplot <<- EOF
	set xlabel "Block Size (Bytes)"
	set ylabel "Throughput (MB/sec)"
	set term png
	set output "plotread.png"
	set title "Read Performance (1 thread)"
	plot "<(sed -n -e 1p -e 9p -e 17p  'result.csv')"  using 5:xticlabels(1) with linespoints title 'Sequential', "<(sed -n -e 2p -e 10p -e 18p 'result.csv')" using 5:xticlabels(1) with linespoints title 'Random'
	set output "plotwrite.png"
	set title "Write Performance (1 thread)"
	set xtics 1,2,3
	plot "<(sed -n -e 3p -e 11p -e 19p  'result.csv')"  using 5:xticlabels(1) with linespoints title 'Sequential', "<(sed -n -e 4p -e 12p -e 20p 'result.csv')" using 5:xticlabels(1) with linespoints title 'Random'

	set xlabel "Threads"
	set yrange [-10:]
	set output "read_by_threads.png"
	set title "Read Performance (Sequential)"
	plot "<(sed -n -e 1p -e 5p 'result.csv')"  using 5:xticlabels(2) with linespoints title '1B', "<(sed -n -e 9p -e 13p 'result.csv')" using 5:xticlabels(2) with linespoints title '1KB', "<(sed -n -e 17p -e 21p 'result.csv')" using 5:xticlabels(2) with linespoints title '1MB'
        set output "write_by_threads.png"
	set yrange [-1:]
	set title "Write Performance (Sequential)"
        plot "<(sed -n -e 4p -e 8p 'result.csv')"  using 5:xticlabels(2) with linespoints title '1B', "<(sed -n -e 12p -e 16p 'result.csv')" using 5:xticlabels(2) with linespoints title '1KB', "<(sed -n -e 20p -e 24p 'result.csv')" using 5:xticlabels(2) with linespoints title '1MB'
EOF
