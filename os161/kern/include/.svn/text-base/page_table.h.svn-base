#ifndef _PAGE_TABLE_H_
#define _PAGE_TABLE_H_

#include <machine/ktypes.h>
#include <machine/types.h>


#define FRAME_NUMBER 0x000fffff	/* mask for getting frame number from PTE */
#define SECOND_INDEX 0x000003ff /* mask for getting index o second level PT */
#define PTE_READONLY 0x1		/* mask for mark PTE as readonly */

enum {
	PTE_FOUND,
	WRITE_PERMITTED, // return when faulttype is readonly type pte found is implicit
	WRITE_NOT_PERMITTED,	//return when faulttype is readonly type pte found is implicit
	NO_SECOND_LEVEL_PT,
	INVALID,
	SWAPPED
};



/// Allocate master (first) level page table and initialize all fields to zero
u_int32_t * pt_create();


int pt_copy(u_int32_t * old_pt, u_int32_t ** new_pt); 


/// when vm_fault happen we pass the fault address to get the frame number correpoding 
/// to that page if frame exists then we fill up *entrylo and return PTE_FOUND  otherwise do these things in fault handler: 
/// if return = NO_SECOND_LEVEL_PT: 1) Allocate second level PT 
///									2) update first_pt entry correponding to the faultaddress
///									3) Allocate a frame for user (if no frame availabe need to schedule swapping laterr)
///									4) read from disk to user frame if needed (mapped region is CODE or DATA);
///									5) Update the second level page table correponding to the faultaddress
///									6) update TLB for faultaddress

/// if return = INVALID: No frame found in memory:  1) Allocate a frame for user if no frame availabe need to schedule swapping
///													2) read from disk to user frame;
///													3) Update the second level page table correponding to the faultaddress
///													4) update TLB for fault addr 
/// if return = SWAPPED: // decide later
int32_t pt_lookup (u_int32_t * page_table , vaddr_t faultaddress, int faulttype, int * entrylo);



void update_first_pt_entry(u_int32_t * page_table, vaddr_t faultaddress, vaddr_t second_pt);


void update_second_pt_entry(u_int32_t * page_table, vaddr_t faultaddress, vaddr_t frameaddress, u_int32_t flags, int * entrylo);


// Take a page table as an argument and make all PTEs to readonly.
void make_readonly(u_int32_t * page_table, int coremap_update);


int pt_copy_on_write (u_int32_t * page_table, vaddr_t faultaddress, int * entrylo);

void pt_deep_destroy(u_int32_t * page_table);

void pt_destroy(u_int32_t * page_table);



#endif /* _PAGE_TABLE_H_ */




