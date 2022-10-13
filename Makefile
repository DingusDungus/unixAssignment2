clean: cleanMatinv cleanKmeans

cleanClient:
	rm -f client ./mathserver/src/*.o

cleanMatinv:
	rm -f matinv

cleanKmeans:
	rm -f kmeans

client: cleanClient
	gcc -Wall ./mathserver/src/fileTransfer.c client.c -o client -g

cleanSeqKmeans:
	rm -f seqkmeans

matinvDebug: cleanMatinv
	gcc -Wall -w -pthread -o matinv matinv-par.c -g -pg

matinv: cleanMatinv
	gcc -Wall -w -O2 -pthread -o matinv matinv-par.c

kmeans: cleanKmeans
	gcc -Wall -w -O2 -pthread -o kmeans kmeans-par.c

kmeansDebug: cleanKmeans
	gcc -Wall -w -O2 -pthread -o kmeans kmeans-par.c -g

kmeanSeq: cleanSeqKmeans
	gcc -Wall -w -O2 -pthread -o seqkmeans kmeans.c

check-kmeans:
	git diff --no-index kmeans-results-correct.txt kmeans-results.txt
