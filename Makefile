all: WTF WTFserv

WTF: clientLibrary.c WTF.c clientHeader.h
	gcc -lssl -lcrypto -o WTF WTF.c clientLibrary.c 

WTFserv: WTFserver.c
	gcc -lpthread -o WTFserver WTFserver.c

test: WTF WTFserv WTFserver.c
	gcc -pthread -o WTFtest WTFtest.c

clean:
	rm -rf WTF 
	rm -rf WTFserver
	rm -rf WTFtest