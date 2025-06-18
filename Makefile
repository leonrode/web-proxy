proxy: main.c utils.c
	gcc -o proxy main.c utils.c

clean:
	rm -f proxy