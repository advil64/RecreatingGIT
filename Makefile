all: test

WTF: clientLibrary.c WTF.c
	gcc -lssl -lcrypto -o WTF WTF.c clientLibrary.c 

WTFserv: WTFserver.c
	gcc -o WTFserver WTFserver.c

test: WTFserver.c clientLibrary.c WTF.c
	gcc -lssl -lcrypto -o WTF WTF.c clientLibrary.c 
	gcc -o WTFserver WTFserver.c
	gcc -o WTFtest WTFtest.c

clean:
	rm -rf WTF 
	rm -rf WTFserver
	rm -rf WTFtest