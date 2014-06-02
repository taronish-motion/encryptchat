#Taronish Daruwalla

p2pim: main.o messagevector.o udp.o
	g++ -Wall -g -ansi -o p2pim main.o messagevector.o udp.o

main.o: main.cpp udp.h messagevector.h
	g++ -Wall -g -ansi -c main.cpp

udp.o: udp.cpp udp.h
	g++ -Wall -g -ansi -c udp.cpp

messagevector.o: messagevector.cpp messagevector.h
	g++ -Wall -g -ansi -c messagevector.cpp

clean:
	rm -f p2pim *.o
