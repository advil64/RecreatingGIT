all: WTF

WTF: clientLibrary.c WTF.c
	gcc -Wall -Werror -lssl -o WTF WTF.c clientLibrary.c 

clean:
	rm -rf WTF