# BU-CS350
Binghamton University - CS350 Operating Systems
Assignment #3 - Page Replacement Algorithms & Workload Generation
Written by: Daniel Bottcher and Matthew VanGorden

Usage: 
	make all => builds executable Prog3Bottcher_dbottch1.exe
	./Prog3Bottcher_dbottch1.exe => runs executable, generating CSV data for the graphing software gnuplot.
		"e_t_data.csv" = eighty/twenty workload data
		"loop_data.csv" = looping workload data
		"nolocal_data.csv" = no locality workload data
		
	
	gnuplot -e "title='Bottcher/VanGorden'" -e "input_filename='name_of_data.csv'" plot_hit_rates.plt > name_of_graph.png
	ex: gnuplot -e "title='Hunger of the Wolf'" -e "input_filename='et_data.csv'" plot_hit_rates.plt > 8020.png

No Locality:
gnuplot -e "title='Bottcher/VanGorden'" -e "input_filename='nolocal_data.csv'" plot_hit_rates.plt > nolocality.png

Eighty/Twenty:
gnuplot -e "title=''Bottcher/VanGorden'" -e "input_filename='et_data.csv'" plot_hit_rates.plt > 8020.png

Looping:
gnuplot -e "title=''Bottcher/VanGorden'" -e "input_filename='nolocal_data.csv'" plot_hit_rates.plt > looping.png


