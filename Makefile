4sh: 4sh.c
	gcc -O2 -Wall -o 4sh 4sh.c

install: 4sh
	cp 4sh /usr/local/bin/
	chmod +x /usr/local/bin/4sh