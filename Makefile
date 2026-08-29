CC ?= cc

all : read-agc get-double-fromagc float-convert float-conv-agc

read-agc : read-agc-memory.c
	$(CC) -o read-agc -Wall read-agc-memory.c -l pthread

get-double-fromagc : get-double-fromagc.c
	$(CC) -o get-double-fromagc -Wall get-double-fromagc.c

float-convert : float-convert.c
	$(CC) -o float-convert -Wall float-convert.c

float-conv-agc : float-conv-agc.c
	$(CC) -o float-conv-agc -Wall float-conv-agc.c

clean :
	rm -f read-agc get-double-fromagc float-convert float-conv-agc
