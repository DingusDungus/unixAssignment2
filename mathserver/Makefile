main:
	gcc -Wall -Werror -c server.c src/*.c
	mv *.o ./objects/

server: main
	gcc ./objects/*.o -o mathserver

clean:
	rm *.o objects/*.o mathserver
