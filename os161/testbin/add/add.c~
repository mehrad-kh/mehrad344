/*
 * Simple program to add two numbers (given in as arguments). Used to
 * test argument passing to child processes.
 *
 * Intended for the basic system calls assignment; this should work
 * once execv() argument handling is implemented.
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

int y = 1;

int
main(int argc, char *argv[])
{
	printf("Y = %d\n",y);
	y = 0;
	printf("Y2 = %d\n",y);
	int i=1, j=2;
	printf("helloooooo  %d\n",argc);
	if (argc!=3) {
		errx(1, "Usage: add num1 num2");
	}
	printf("i get this %s and %s\n",argv[1],argv[2]);
	i = atoi(argv[1]);
	j = atoi(argv[2]);

	printf("Answer: %d\n", i+j);

	return 0;
}
