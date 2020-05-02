all: WTF WTFserv

WTF: clientLibrary.c WTF.c
	gcc -lssl -lcrypto -o WTF WTF.c clientLibrary.c 

WTFserv: WTFserver.c
	gcc -o WTFserver WTFserver.c

clean:
	rm -rf WTF 
	rm -rf WTFserver