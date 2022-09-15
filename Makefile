clean:
	rm matinv kmeans

matinv:
	gcc -w -O2 -o matinv matrix_inverse.c

kmeans:
	gcc -w -O2 -o kmeans kmeans.c
