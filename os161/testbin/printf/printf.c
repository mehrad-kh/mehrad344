#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
		
     
        int i = 0;
        /*for (i=0; i <1000; i++)
        	printf("the number is %d", i); */
        if (argc != 1) {
                errx(1, "Usage: add num1 num2");
        }

        printf("printf works!\n");
        printf("Enter your character:");

        int c = getchar();
        printf("The character read %d\n",c); 

        int a = 2;
        int b =3;
        printf("Now going to add %d with %d\n", a, b);
        printf ("the result is %d\n",a+b);

	return 0;
}
