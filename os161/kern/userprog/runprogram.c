
/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>
 #include <vnode.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, unsigned long  argc, char ** argv)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
	char **ARGS;


	kprintf("the program name is %s\n",progname);
	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	curthread->t_vmspace->v = v;
	VOP_INCREF(curthread->t_vmspace->v);

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}
	/* Define the heapbase and top*/
	// Page alligned heapbase
	result = as_define_heap(curthread->t_vmspace, &curthread->t_vmspace->as_heapbase);

	if (result) {
		
		return result;
	}
	curthread->t_vmspace->as_heapvtop=curthread->t_vmspace->as_heapbase + PAGE_SIZE;

	/* Warp to user mode. */
	int i=0;
	int sum=0;
	int actual=0;
	int pointers[16];

	/// Preparing the argument's methadata
	ARGS = (char**) kmalloc(sizeof(char*)*16);
	for (i=0;i<argc;i++) 
	{
		ARGS[i]=(char*) kmalloc( sizeof(char)*(strlen(argv[i])+1));

		strcpy(ARGS[i],argv[i]);
		
	}

	//kprintf("we are here\n");
	curthread->args=ARGS;
	curthread->argc=argc;
	
	//preparing the stack

	for (i=0;i<argc;i++) // total number of characters
		sum+=strlen(argv[i])+1;
  	stackptr-=(argc+1)* sizeof (vaddr_t)+sum;
  	

	
	md_usermode(argc/*argc*/, stackptr+sum /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}
