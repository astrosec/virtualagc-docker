all : read-agc get-double-fromagc float-convert

read-agc : read-agc-memory.c
	clang -o read-agc -Wall read-agc-memory.c -l pthread

get-double-fromagc : get-double-fromagc.c
	clang -o get-double-fromagc -Wall get-double-fromagc.c

float-convert : float-convert.c
	clang -o float-convert -Wall float-convert.c

clean : 
	rm read-agc get-double-fromagc float-convert
