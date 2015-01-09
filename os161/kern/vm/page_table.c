#include <kern/errno.h>
#include <page_table.h>
#include <machine/vm.h>
#include <machine/tlb.h>
#include <elf.h>
#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <machine/coremap.h>
#include <vm.h>



u_int32_t * pt_create()
{
	u_int32_t * addr = (u_int32_t *) kmalloc(4096);

	if (addr)
		bzero(addr, 4096);
	else
		return NULL;

	return addr;
}


int pt_copy(u_int32_t * old_pt, u_int32_t ** new_pt)
{
	int i;
	vaddr_t old_second_pt;
	vaddr_t new_second_pt;

	*new_pt = (u_int32_t *) kmalloc(4096);

	if (*new_pt == NULL) {
		return ENOMEM;
	}
	bzero(*new_pt, 4096);

	for (i = 0; i < 1024; i++){
		if (old_pt[i] != 0){

			old_second_pt = PADDR_TO_KVADDR(old_pt[i]);
			// Allocate new_second_pt 
			new_second_pt = (vaddr_t) kmalloc(4096);
			//paddr_t temp = new_second_pt - 0x80000000;
			//kprintf("**************temp is %x\n",temp);
			bzero((void *)new_second_pt, 4096);

			if (new_second_pt == NULL) {
				return ENOMEM;
			}

			// Update new_pt entry
			//paddr_t temp = new_second_pt - 0x80000000;
			(*new_pt)[i] = (paddr_t)KVADDR_TO_PADDR(new_second_pt);
			//kprintf("pt_copy 0x%x\n",(*new_pt)[i]);

			// Copy old_second_pt into new_second_pt
			memmove((void *)new_second_pt, (const void *)old_second_pt, PAGE_SIZE);

		}
	}
	return 0;

}


/// when vm_fault happen we pass the fault address to get the frame number correpoding 
/// to that page if frame exists then we return frame number otherwise do these things in fault handler: 
/// if return = NO_SECOND_LEVEL_PT: 1) Allocate second level PT 
///									2) update first_pt entry correponding to the faultaddress
///									3) Allocate a frame for user
///									4) read from disk to user frame;
///									5) Update the second level page table correponding to the faultaddress
///									6) update TLB for fault addr

/// if return = INVALID: No frame found in memory:  1) Allocate a frame for user if no frame availabe need to schedule swapping
///													2) read from disk to user frame;
///													3) Update the second level page table correponding to the faultaddress
///													4) update TLB for fault addr 
/// if return = SWAPPED: // decide later
int32_t pt_lookup (u_int32_t * page_table , vaddr_t faultaddress, int faulttype,  int * entrylo)
{
	//assert (page_table);

	// Calculate index to first page table from faultaddress
	u_int32_t index_1 = faultaddress >> 22;

	if (page_table[index_1] == 0)
		return NO_SECOND_LEVEL_PT;

	// Here we have valid frame number for second level pt
	// so we calculate the kernel virtual address of second level pt
	paddr_t psecond_pt = page_table[index_1];
	u_int32_t * second_pt = (u_int32_t *) PADDR_TO_KVADDR(psecond_pt); 

	// Calculate index to second page table from faultaddress
	u_int32_t index_2 = (faultaddress >> 12) & SECOND_INDEX;

	// Now read the PTE and check if it is valid 
	int valid = second_pt[index_2] & TLBLO_VALID;

	if (valid)
	{
		if (faulttype == VM_FAULT_READONLY )
		{
			*entrylo = second_pt[index_2] & (~PTE_READONLY);
			if (second_pt[index_2] & TLBLO_DIRTY)
				return WRITE_PERMITTED;
			else
				return WRITE_NOT_PERMITTED;
		}

		else
		{
			if (second_pt[index_2] & PTE_READONLY)
				*entrylo = second_pt[index_2] & (~TLBLO_DIRTY);

			else
				 *entrylo = second_pt[index_2];

			return PTE_FOUND;
		}	
	}

	else
	{
		int swapped = 0;
		if (swapped)
			return SWAPPED;
		else
			return INVALID;
	}

}




void update_first_pt_entry(u_int32_t * page_table, vaddr_t faultaddress, vaddr_t second_pt)
{
	u_int32_t index_1;
	paddr_t frameaddress;

	// Calculate index to first page table from faultaddress
	index_1 = faultaddress >> 22;

	//Calculate the frame number associated to second_pt and store it in page_table[index_1]
	frameaddress = (KVADDR_TO_PADDR(second_pt)); 

	page_table[index_1] = 0;
	page_table[index_1] = frameaddress;
	//kprintf("update_first_pt_entry 0x%x index =%d\n",page_table[index_1], index_1);

}


void update_second_pt_entry(u_int32_t * page_table, vaddr_t faultaddress, vaddr_t frameaddress, u_int32_t flags, int * entrylo)
{

	// Calculate index to first page table from faultaddress
	u_int32_t index_1 = faultaddress >> 22;

	paddr_t psecond_pt = page_table[index_1];
	u_int32_t * second_pt = (u_int32_t *) PADDR_TO_KVADDR(psecond_pt); 

	// Calculate index to second page table from faultaddress
	u_int32_t index_2 = (faultaddress >> 12) & SECOND_INDEX;

	frameaddress = (paddr_t)(KVADDR_TO_PADDR(frameaddress));
	second_pt[index_2] = 0;
	second_pt[index_2] = frameaddress;
	//kprintf("update_second_pt_entry 0x%x index %d \n",second_pt[index_2], index_2);

	// Now set the control bits

	//set valid bits
	second_pt[index_2] = (second_pt[index_2] | TLBLO_VALID);

	//set writable bits if the page is writable the permission can be determined by checking flags
	if ((flags & PF_W)){
		second_pt[index_2] = (second_pt[index_2] | TLBLO_DIRTY);
	}

	*entrylo = second_pt[index_2];
}


void make_readonly(u_int32_t * page_table, int coremap_update)
{
	int i, j;
	u_int32_t * second_pt;

	for (i = 0; i < 1024; i++) 
	{

		if (page_table[i] != 0) 
		{
			second_pt = PADDR_TO_KVADDR(page_table[i]);
			for (j = 0; j < 1024; j++)
			{
				if (second_pt[j] & TLBLO_VALID)
				{	
					if (!coremap_update);
						//kprintf("parent &&&&&&&& 0x%x index = %d\n", (second_pt[j] & TLBLO_PPAGE), j);
					else
						//kprintf("child &&&&&&&& 0x%x index = %d\n", (second_pt[j] & TLBLO_PPAGE), j);
					second_pt[j] = second_pt[j] | PTE_READONLY;
					if (coremap_update)
					{
						//kprintf("&&&&&&&& 0x%x index = %d\n", (second_pt[j] & TLBLO_PPAGE), j);
						increfcount(second_pt[j] & TLBLO_PPAGE);
					}
				}

			}

		}

	}
}



int pt_copy_on_write (u_int32_t * page_table, vaddr_t faultaddress, int * entrylo)
{
	/*
	* General overview of what needs to be done:
	* 1) Go to my page table and get the current frame that faultaddress is mapped to.
	* 2) Check whether that frame is shared or not
	* 3) if not shared then get rid of the readonly bit in the pte. 
	* 4) if shared then first decrement reference count of the frame
	*    then allocate a new frame, copy the content of old frame into the new one,
	     update the pte accordingly and the return. 
	*/

	//kprintf("doing ccccccoooooppppy  ooonnnn wriiite \n");

	// Calculate index to first page table from faultaddress
	u_int32_t index_1 = faultaddress >> 22;

	// Here we have valid frame number for second level pt
	// so we calculate the kernel virtual address of second level pt
	paddr_t psecond_pt = page_table[index_1];
	u_int32_t * second_pt = (u_int32_t *) PADDR_TO_KVADDR(psecond_pt); 

	// Calculate index to second page table from faultaddress
	u_int32_t index_2 = (faultaddress >> 12) & SECOND_INDEX;

	paddr_t frameaddr = second_pt[index_2] & TLBLO_PPAGE;

	if (isShared (frameaddr))
	{
		decrefcount(frameaddr);
		vaddr_t newframeaddr = (vaddr_t)kmalloc(PAGE_SIZE);
		if (!newframeaddr){
			return ENOMEM;
		}

		memmove((void *)newframeaddr, (const void *) (PADDR_TO_KVADDR(frameaddr)), PAGE_SIZE);
		//memcpy((void *)newframeaddr, (const void *) (PADDR_TO_KVADDR(frameaddr)), PAGE_SIZE);

		second_pt[index_2] = KVADDR_TO_PADDR(newframeaddr) | TLBLO_DIRTY | TLBLO_VALID;
		//kprintf("copy on write 0x%x \n ",second_pt[index_2]);

		*entrylo = second_pt[index_2];
		return 0;

	}

	else
	{
		second_pt[index_2] = second_pt[index_2] & (~PTE_READONLY);
		*entrylo = second_pt[index_2];
		return 0;
	}


}



void pt_deep_destroy(u_int32_t * page_table)
{
	int i, j;
	u_int32_t * second_pt;
	paddr_t frameaddr;
	/* General overview of what needs to be done
	* 1) Go through the first level page table
	* 2) Find all second level pages tables
	* 3) Go through second level page table entries and check if they are valid
	* 4) if they are valid following needs to be done: i) Find the frame they are pointing to
	*												   ii) if frame is shared then just decrement the refcount
	*												   iii) otherwise free the frame
    *
	* 5) then free the second level page table
	* 6) after all free the first level page table 
	*/

	assert(page_table != NULL)

	for (i = 0; i < 1024; i++)
	{
		if (page_table[i] != 0)
		{
			second_pt = PADDR_TO_KVADDR(page_table[i]);

			for (j = 0; j < 1024; j++)
			{
				if (second_pt[j] & TLBLO_VALID)
				{
					frameaddr = second_pt[j] & TLBLO_PPAGE;
					if (isShared (frameaddr))
					{
						// just decrement reference count
						decrefcount(frameaddr);

					}

					else
					{
						// We need to free the frame since this was the last process using it
						kfree (PADDR_TO_KVADDR(frameaddr));
					}

				}
			}
			kfree(second_pt);
		}
	}
	// Now free the first level page table
	kfree(page_table);
}



/* just destory page tables */
void pt_destroy(u_int32_t * page_table)
{
	int i;
	u_int32_t * second_pt;

	for (i = 0; i < 1024; i++)
	{
		if (page_table[i] != 0 )
		{
			second_pt = PADDR_TO_KVADDR(page_table[i]);
			kfree(second_pt);
		}
	}
	kfree(page_table);
}
