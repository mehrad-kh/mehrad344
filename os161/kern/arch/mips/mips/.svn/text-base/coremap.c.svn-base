#include <machine/coremap.h>



static int COREMAP_SIZE = 0;		/* Number of frames we are to manage in physical memory */

static u_int32_t FRAME0_ADDR;		/* Address of frame correpoding to coremap[0] frame manged by coremap i.e 51 */ 

static int UNALLOCATED_FRAME; 		/* Keeps track of number of unallocated frames */
static int KERNEL_FRAME; 			/* Number of kernel frames */
static int USER_FRAME;				/* Number of user frames */

struct frame_info * coremap;


int VM_INITIALIZED;


void coremap_initialization ()
{
	UNALLOCATED_FRAME = 0;
	KERNEL_FRAME = 0;
	USER_FRAME = 0;
	u_int32_t lo; 
	u_int32_t hi;
	int free_frames_num;
	ram_getsize(&lo, &hi);
	hi -= PAGE_SIZE;

	// Finding number of free frames
	free_frames_num = ((hi-lo)/PAGE_SIZE)+1;

	// sizeof(frame_info) * COREMAP_SIZE + COREMAP_SIZE * PAGE_SIZE = free_frames_num * PAGE_SIZE
	COREMAP_SIZE = free_frames_num * PAGE_SIZE/(PAGE_SIZE + sizeof(struct frame_info));

	int npages = (COREMAP_SIZE * sizeof(struct frame_info)/PAGE_SIZE) + 1;
	//vaddr_t addr = alloc_kpages(npages);

	//coremap = (struct frame_info *)kmalloc(COREMAP_SIZE * sizeof(struct frame_info));
	coremap = (struct frame_info *)kmalloc(npages * PAGE_SIZE);

	bzero(coremap, COREMAP_SIZE * sizeof (struct frame_info));

	ram_getsize(&lo, &hi);
	FRAME0_ADDR = lo;
	UNALLOCATED_FRAME = COREMAP_SIZE;
	KERNEL_FRAME = 0;
	USER_FRAME = 0;

	//******
	// paddr_t a = allocate_frame(5);
	//free_frame(PADDR_TO_KVADDR(a));
	VM_INITIALIZED = 1;
}



void coremap_printstats()
{
	int spl = splhigh();

	kprintf ("***********Coremape Status Is**************** \n");
	kprintf ("UNALLOCATED FRAMES: %d\nKERNEL FRAME: %d\nUSER FRAME: %d\n",
	 UNALLOCATED_FRAME, KERNEL_FRAME, USER_FRAME);
	

	splx(spl);
}











/*
* Aloocate n frame of memory, retrun physical address of first frame
*/
paddr_t allocate_frame(int n)
{
	
	int i, j, index;
	int found = 0;
	//coremap[2].status = 1;
	//coremap[8].status =1; 

	int spl = splhigh();
	if (n > UNALLOCATED_FRAME)
	{
		//kprintf("OH NO ALLOCATION for %d frame\n",n);
		splx(spl);
		return 0;
	}

	i = 0;
	while (i < COREMAP_SIZE-(n-1))
	{	
		for (j = 0; j < n; j++)
			if (coremap[i+j].status != 0)
				break;

		if (j == n)
		{
			index = i;
			found = 1;
			break;
		}
		i += j+1;
	}

	if (found == 1)
	{
		int k;
		int allocation_size = n;
		for (k = index; k < index+n; k++)
		{
			coremap[k].status = KERNEL_ALLOCATED;
			coremap[k].alloc_size = allocation_size;
			allocation_size--;
		}
		UNALLOCATED_FRAME -=n;
		KERNEL_FRAME += n;
		//kprintf("allocting %d frame ***** unallocated frame == %d\n",n, UNALLOCATED_FRAME);
		
		paddr_t temp = FRAME0_ADDR + index * PAGE_SIZE;
		//kprintf("address retuning by alloc_frame is 0x%x\n",FRAME0_ADDR + index * PAGE_SIZE );
		splx(spl);
		return (FRAME0_ADDR + index * PAGE_SIZE);
	}

	//kprintf("OH NO ALLOCATION\n");
	splx(spl);
	return 0;

}

/*
* Free block of frames, starting at frame
*/
int callz=0;
int free_frame(vaddr_t addr)
{
	//kprintf("in free frame\n");
	int spl = splhigh();
	//kprintf("In free_frame callz=%d\n",callz++);

	paddr_t address = addr-MIPS_KSEG0;
	//kprintf("paddr_t = %x\n", address);
	//if (address % PAGE_SIZE != 0){
	//	kprintf("************the address is %x",address);
	//	return 0;
	//}
	//kprintf("in free frame\n");
	int frame_index = (address - FRAME0_ADDR)/PAGE_SIZE;

	int n = 0;
	while (frame_index < COREMAP_SIZE)
	{

		n++;
		//kprintf("here 0.0\n");
		if (coremap[frame_index].alloc_size == 1)
		{
			//kprintf("here 0.2a\n");
			bzero(coremap+frame_index, sizeof (struct frame_info));
			//kprintf("here 0.2b\n");
			break;
		}
        //kprintf("here 0.3a\n");
		bzero(coremap+frame_index, sizeof (struct frame_info));
		//kprintf("here 0.3b\n");
		frame_index++;
	}

	UNALLOCATED_FRAME +=n;
	KERNEL_FRAME -= n;
	//kprintf("deallocting %d frame \n",n);
    //kprintf("exiting freeframe\n");
    //kprintf("returning from free frame\n");
	splx(spl); 
	return 0; 
}

/*
* Map physical address (frame) to virtual address (page_nr, pid)
* NOTE: a frame may be mapped to multiple pages in the same or 
* Different address spaces.
*/
int map (int pid, int page_nr, int frame)
{

}


/*
* Removes mapping of frame corresponding to pgae_nr from address space
*/
int unmap (int pid, int page_nr)
{

}

void increfcount(paddr_t frameaddr)
{
	int index;

	index = (frameaddr - FRAME0_ADDR)/PAGE_SIZE;
	//kprintf("Im here increcout with index %d and frameaddr 0x%x\n", index, frameaddr);
	coremap[index].shared += 1; 
	//kprintf("Im here increcoutffff\n");
}


void decrefcount(paddr_t frameaddr)
{
	int index;

	index = (frameaddr - FRAME0_ADDR)/PAGE_SIZE;
	coremap[index].shared -= 1; 
}



int isShared (paddr_t frameaddr)
{
	int index;

	index = (frameaddr - FRAME0_ADDR)/PAGE_SIZE;
	return coremap[index].shared;
}
