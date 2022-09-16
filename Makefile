clean: cleanMatinv cleanKmeans

cleanMatinv:
	rm -f matinv

cleanKmeans:
	rm -f kmeans

matinv: cleanMatinv
	gcc -Wall -w -O2 -lpthread -o matinv matrix_inverse.c

kmeans: cleanKmeans
	gcc -Wall -w -O2 -lpthread -o kmeans kmeans.c
