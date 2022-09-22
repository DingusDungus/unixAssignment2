clean: cleanMatinv cleanKmeans

cleanMatinv:
	rm -f matinv

cleanKmeans:
	rm -f kmeans

matinvDebug: cleanMatinv
	gcc -Wall -w -lpthread -o matinv matrix_inverse_parallel.c -g

matinv: cleanMatinv
	gcc -Wall -w -O2 -lpthread -o matinv matrix_inverse_parallel.c

kmeans: cleanKmeans
	gcc -Wall -w -O2 -lpthread -o kmeans kmeans.c
