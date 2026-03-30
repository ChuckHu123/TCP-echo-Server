.PHONY: all clean helloworld TCP_V1 TCP_V2 TCP_V3 TCP_V4

all:

clean:
	rm -f build/TCP_V1 build/TCP_V1_comment build/TCP_V2 build/hello build/TCP_V3 build/TCP_V4

TCP_V1:	TCP_V1.c
	mkdir -p build
	gcc -Wall TCP_V1.c -o build/TCP_V1
	gcc -Wall TCP_V1_comment.c -o build/TCP_V1_comment

TCP_V2: TCP_V2.c
	mkdir -p build
	gcc -Wall TCP_V2.c -o build/TCP_V2 -lpthread

TCP_V3: TCP_V3.c
	mkdir -p build
	gcc -Wall TCP_V3.c -o build/TCP_V3

TCP_V4: TCP_V4.c
	mkdir -p build
	gcc -Wall TCP_V4.c -o build/TCP_V4

TCP_Client_V1: TCP_Client_V1.c
	mkdir -p build
	gcc -Wall TCP_Client_V1.c -o build/TCP_Client_V1

helloworld:	hello.c
	mkdir -p build
	gcc hello.c -o build/hello
