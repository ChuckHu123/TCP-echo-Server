.PHONY: all clean helloworld TCP_Server_V1 TCP_Server_V2 TCP_Server_V3 TCP_Server_V4 TCP_Server_V5

all:

clean:
	rm -f build/TCP_Server_V1 build/TCP_Server_V1_comment \
	build/TCP_Server_V2 build/hello build/TCP_Server_V3 build/TCP_Server_V4 build/TCP_Server_V5 \
	build/TCP_Client_V1 build/TCP_Client_V2

TCP_Server_V1:	TCP_Server_V1.c
	mkdir -p build
	gcc -Wall TCP_Server_V1.c -o build/TCP_Server_V1
	gcc -Wall TCP_Server_V1_comment.c -o build/TCP_Server_V1_comment

TCP_Server_V2: TCP_Server_V2.c
	mkdir -p build
	gcc -Wall TCP_Server_V2.c -o build/TCP_Server_V2 -lpthread

TCP_Server_V3: TCP_Server_V3.c
	mkdir -p build
	gcc -Wall TCP_Server_V3.c -o build/TCP_Server_V3

TCP_Server_V4: TCP_Server_V4.c
	mkdir -p build
	gcc -Wall TCP_Server_V4.c -o build/TCP_Server_V4

TCP_Server_V5: TCP_Server_V5.c
	mkdir -p build
	gcc -Wall TCP_Server_V5.c -o ./build/TCP_Server_V5 -levent

TCP_Client_V1: TCP_Client_V1.c
	mkdir -p build
	gcc -Wall TCP_Client_V1.c -o build/TCP_Client_V1

TCP_Client_V2: TCP_Client_V2.c
	mkdir -p build
	gcc -Wall TCP_Client_V2.c -o build/TCP_Client_V2

helloworld:	hello.c
	mkdir -p build
	gcc hello.c -o build/hello
