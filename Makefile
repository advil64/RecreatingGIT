all: WTF

WTF: clientLibrary.c WTF.c
	gcc -Werror -lssl -lcrypto -o WTF WTF.c clientLibrary.c 

clean:
	rm -rf WTF