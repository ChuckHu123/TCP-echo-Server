TCP_V1: TCP_V1.c
	gcc -Wall TCP_V1.c -o TCP_V1
	gcc -Wall TCP_V1_comment.c -o TCP_V1_comment

helloworld:	hello.c
	gcc hello.c -o hello