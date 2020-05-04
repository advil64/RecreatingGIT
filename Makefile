all: WTF WTFserv

WTF: clientLibrary.c WTF.c clientHeader.h
	gcc -lssl -lcrypto -o WTF WTF.c clientLibrary.c 

WTFserv: WTFserver.c
	gcc -o WTFserver WTFserver.c

<<<<<<< HEAD
test: WTF WTFserv WTFserver.c
=======
test: WTF WTFserv WTFtest.c
>>>>>>> 6d3993dfa59424f5d550ad81aaf9658dc5e1346b
	gcc -pthread -o WTFtest WTFtest.c

clean:
	rm -rf WTF 
	rm -rf WTFserver
	rm -rf WTFtest