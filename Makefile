#Taronish Daruwalla

p2pim: main.o messagevector.o udp.o tcp.o EncryptionLibrary.o
	g++ -Wall -g -ansi -o p2pim main.o messagevector.o udp.o tcp.o EncryptionLibrary.o

main.o: main.cpp udp.h tcp.h messagevector.h EncryptionLibrary.h
	g++ -Wall -g -ansi -c main.cpp

udp.o: udp.cpp udp.h
	g++ -Wall -g -ansi -c udp.cpp

tcp.o: tcp.cpp tcp.h
	g++ -Wall -g -ansi -c tcp.cpp

messagevector.o: messagevector.cpp messagevector.h
	g++ -Wall -g -ansi -c messagevector.cpp

EncryptionLibrary.o: EncryptionLibrary.cpp EncryptionLibrary.h
		g++ -Wall -g -ansi -c EncryptionLibrary.cpp

clean:
	rm -f p2pim *.o
