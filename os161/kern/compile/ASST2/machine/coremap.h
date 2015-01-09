#ifndef _MACHINE_COREMAP_H_
#define _MACHINE_COREMAP_H_

#include <types.h>
#include <machine/vm.h>
#include <vm.h>

struct page_info {
	int page_nr;
	int pid;
};

struct frame_info {
	u_int8_t status; 
	u_int8_t shared;
	u_int8_t protection;
	u_int8_t alloc_size;
	struct page_info * page_list;
};


enum status 
{
	UNALLOCATED,
	KERNEL_ALLOCATED,
	USER_ALLOCATED
};

enum protection
{
	READ_ONLY,
	WRITABLE,
	TEMP_READ_ONLY
};




/* Function for setting up the coremap */

void coremap_initialization ();



void coremap_printstats();


/* Coremap API */
/*
* Aloocate n frame of memory, retrun physical address of first frame
*/
paddr_t allocate_frame(int n);

/*
* Free block of frames, starting at frame
*/
int free_frame(vaddr_t addr);

/*
* Map physical address (frame) to virtual address (page_nr, pid)
* NOTE: a frame may be mapped to multiple pages in the same or 
* Different address spaces.
*/
int map (int pid, int page_nr, int frame);


/*
* Removes mapping of frame corresponding to pgae_nr from address space
*/
int unmap (int pid, int page_nr);


/*
*
*/
void increfcount(paddr_t frameaddr);

void decrefcount(paddr_t frameaddr);

int isShared (paddr_t frameaddr);



#endif /* _MACHINE_COREMAP_H_ */