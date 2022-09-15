clean:
	rm matinv kmeans

matinv:
	gcc -w -O2 -lpthread -o matinv matrix_inverse.c

kmeans:
	gcc -w -O2 -lpthread -o kmeans kmeans.c
