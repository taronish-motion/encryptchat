#Taronish Daruwalla

p2pim: main.o messagevector.o udp.o tcp.o
	g++ -Wall -g -ansi -o p2pim main.o messagevector.o udp.o tcp.o

main.o: main.cpp udp.h tcp.h messagevector.h
	g++ -Wall -g -ansi -c main.cpp

udp.o: udp.cpp udp.h
	g++ -Wall -g -ansi -c udp.cpp

tcp.o: tcp.cpp tcp.h
	g++ -Wall -g -ansi -c tcp.cpp

messagevector.o: messagevector.cpp messagevector.h
	g++ -Wall -g -ansi -c messagevector.cpp

clean:
	rm -f p2pim *.o
