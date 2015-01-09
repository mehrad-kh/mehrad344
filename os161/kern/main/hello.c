#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <hello.h>

void hello(void)
{
	kprintf("Hello World\n");
}
