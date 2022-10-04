main:
	gcc -c server.c src/*.c
	mv *.o ./objects/

server: main
	gcc ./objects/*.o -o mathserver

clean:
	rm -f *.o objects/*.o mathserver resultFile*
