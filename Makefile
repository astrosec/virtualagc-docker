float-conv-agc : float-conv-agc.o 
	gcc -o float-conv-agc float-conv-agc.o -lm
float-conv-agc.o : float-conv-agc.c 
	gcc -c float-conv-agc.c 
clean :
	rm float-conv-agc float-conv-agc.o 

