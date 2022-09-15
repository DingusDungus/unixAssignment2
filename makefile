main:
	gcc -Wall -Werror -c server.c src/*.c
	mv *.o ./objects/

server: main
	gcc ./objects/*.o -o mathserver

clean:
	rm *.o objects/*.o mathserver matinv kmeans

matinv:
	gcc -w -O2 -o matinv matrix_inverse.c

kmeans:
	gcc -w -O2 -o kmeans kmeans.c
