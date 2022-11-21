all: clean matinv kmeans client server

clean: cleanMatinv cleanKmeans cleanClient cleanServer

cleanServer:
	rm -f ./mathserver/kmeans_client* ./mathserver/matinv_client* ./mathserver/*.o ./mathserver/objects/*.o ./mathserver/mathserver

cleanClient:
	rm -f client ./mathserver/objects/*.o kmeans_client* matinv_client*

cleanMatinv:
	rm -f matinv

cleanKmeans:
	rm -f kmeans

server:
	gcc -c ./mathserver/server.c ./mathserver/src/*.c
	mv *.o ./mathserver/objects/
	gcc ./mathserver/objects/*.o -o ./mathserver/mathserver -g

client:
	gcc -Wall ./mathserver/src/fileTransfer.c ./mathserver/src/clientArgsParsing.c ./mathserver/src/clientErrorChecking.c ./mathserver/src/helperFunctions.c client.c -o client -g

matinvDebug:
	gcc -Wall -w -pthread -o matinv matinv-par.c -g -pg

matinv:
	gcc -Wall -w -O2 -pthread -o matinv matinv-par.c

kmeans: cleanKmeans
	gcc -Wall -w -O2 -pthread -o kmeans kmeans-par.c

kmeansDebug:
	gcc -Wall -w -O2 -pthread -o kmeans kmeans-par.c -g

kmeanSeq:
	gcc -Wall -w -O2 -pthread -o seqkmeans kmeans.c

check-kmeans:
	git diff --no-index kmeans-results-correct.txt kmeans-results.txt
