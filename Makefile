clean:
	rm matinv kmeans

matinv:
	rm matinv
	gcc -w -O2 -o matinv matrix_inverse.c

kmeans:
	rm kmeans
	gcc -w -O2 -o kmeans kmeans.c
