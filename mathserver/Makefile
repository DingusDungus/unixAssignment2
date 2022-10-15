main: clean
	gcc -c server.c src/*.c
	mv *.o ./objects/

server: main
	gcc ./objects/*.o -o mathserver -g

clean:
	rm -f *.o objects/*.o mathserver resultFile* inFile* outFile* ../*.o ../clientResultFile.txt
