clean: cleanMatinv cleanKmeans

cleanClient:
	rm -f client ./mathserver/src/*.o

cleanMatinv:
	rm -f matinv

cleanKmeans:
	rm -f kmeans

client: cleanClient
	gcc -Wall ./mathserver/src/fileTransfer.c client.c -o client

matinvDebug: cleanMatinv
	gcc -Wall -w -pthread -o matinv matrix_inverse_parallel.c -g

matinv: cleanMatinv
	gcc -Wall -w -O2 -pthread -o matinv matrix_inverse_parallel.c

kmeans: cleanKmeans
	gcc -Wall -w -O2 -pthread -o kmeans kmeans-multithreading.c

kmeansDebug: cleanKmeans
	gcc -Wall -w -O2 -pthread -o kmeans kmeans-multithreading.c -g

kmeanSeq: cleanKmeans
	gcc -Wall -w -O2 -pthread -o kmeans kmeans.c

check-kmeans:
	git diff --no-index kmeans-results-correct.txt kmeans-results.txt
