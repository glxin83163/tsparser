# tsparser
a small tool for ts packets parser

Compile:
	make clean;make

Usage: ./tsparser [s] -i filename

	switches:
	-i filename - opens "filename"
	
	-o outfile  - writes to "outfile"
	
	-c cfgfile  - config file 
	
	-d display ts Parser info!
	
	-p psi_type  - writes psipid info to "outfile"
	
	   psi_type : all-default(0)|pat(1)|pmt(2)|cat(4)|nit(8)

                      bat(32)|sdt(64)|eit(256)|tot(1024)|tdt(2048)

Tool features:

First:
    the Tool can be display SI information int the ts packets
	example:
	
	./tsparser -di xxxx.ts -p 1
	
	display PAT information

Second:
	the Tool can be used to capture the data or remove the data form ts file. You need to used configure file.
	example:
	
	./tsparser -i xxxx.ts -c cfg.ini

Note:
	Command line config is high priority on the display information command
