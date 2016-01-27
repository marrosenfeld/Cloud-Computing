gcc -o helloworld hello_world.c
./helloworld
gnuplot <<- EOF
	set xlabel "Label"
        set ylabel "Label2"
        set term png
        set output "x.png"
        plot sin(x)/x
EOF




