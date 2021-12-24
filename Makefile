all: shell

shell : shell.c
	gcc -g -Wall -Wvla -Werror  -fsanitize=address -std=c11 shell.c -o shell
clean:
	rm -f shell