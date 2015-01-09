#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <machine/coremap.h>
#include <page_table.h>
#include <uio.h>
#include <vnode.h>


/*
 * Dumb MIPS-only "VM system" that is intended to only be just barely
 * enough to struggle off the ground. You should replace all of this
 * code while doing the VM assignment. In fact, starting in that
 * assignment, this file is not included in your kernel!
 */

/* under dumbvm, always have 48k of user stack */
#define DUMBVM_STACKPAGES    12

 enum mapping {
 	CODE,
 	DATA,
 	HEAP,
 	STACK,
 	UNMAPPED
 };

 extern int VM_INITIALIZED;

void
vm_bootstrap(void)
{
	/* Do nothing. */
	/*u_int32_t lo; 
	u_int32_t hi;
	ram_getsize(&lo, &hi);*/
	coremap_initialization ();
	return;
}

static
paddr_t
getppages(unsigned long npages)
{
	int spl;
	paddr_t addr;

	spl = splhigh();

	if (! VM_INITIALIZED)
		addr = ram_stealmem(npages);
	else
		addr = allocate_frame(npages);
	//addr = ram_stealmem(npages);
	
	splx(spl);
	return addr;
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t 
alloc_kpages(int npages)
{
	paddr_t pa;
	pa = getppages(npages);
	if (pa==0) {
		return 0;
	}
	return PADDR_TO_KVADDR(pa);
}

void 
free_kpages(vaddr_t addr)
{
	/* nothing */

	if (VM_INITIALIZED)
		free_frame(addr);
	
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	vaddr_t vbase1, vtop1, vbase2, vtop2, heapbase, heaptop, stackbase, stacktop, temp;
	paddr_t paddr;
	int i, result;
	int mapped_region;
	u_int32_t ehi, elo;
	struct addrspace *as;
	u_int32_t entrylo = 0;
	vaddr_t newframe = 0;
	u_int32_t * second_pt;
	u_int32_t flags = 0;

	int spl;

	spl = splhigh();
	
	//faultaddress &= PAGE_FRAME;

	/*DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

	switch (faulttype) {
	    case VM_FAULT_READONLY:
		//We always create pages read-write, so we can't get this 
		panic("dumbvm: got VM_FAULT_READONLY\n");
	    case VM_FAULT_READ:

	   	case VM_FAULT_WRITE:

		break;
	    default:
		splx(spl);
		return EINVAL;
	}*/

	as = curthread->t_vmspace;
	if (as == NULL) {
		/*
		 * No address space set up. This is probably a kernel
		 * fault early in boot. Return EFAULT so as to panic
		 * instead of getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

	/* Assert that the address space has been set up properly. */
	for (i = 0 ; i < as->as_segnum; i++){
		assert(as->as_region[i].p_vaddr != 0);
		assert((as->as_region[i].p_vaddr & PAGE_FRAME) == as->as_region[i].p_vaddr);
	}

	assert(as->as_stackvbase != 0);
	assert(as->as_heapvtop != 0);



	// Check if faultaddress is in the mapped region 
	// I need to get rid of the stack afterward since stack size is no longer fixed

	vbase1 = as->as_region[0].p_vaddr;
	vtop1 = vbase1 + as->as_region[0].p_memsz;
	vbase2 = as->as_region[1].p_vaddr;
	vtop2 = vbase2 + as->as_region[1].p_memsz;
	//heapbase = vbase2 + as->as_region[1].p_memsz;
	heapbase= as->as_heapbase;
	heaptop = as->as_heapvtop;
	stackbase = as->as_stackvbase - PAGE_SIZE;
	stacktop = USERSTACK;


	if (faultaddress >= vbase1 && faultaddress < vtop1) {
		mapped_region = CODE;
	}
	else if (faultaddress >= vbase2 && faultaddress < vtop2) {
		mapped_region = DATA;
	}
	else if (faultaddress >= heapbase && faultaddress < heaptop) {
		mapped_region = HEAP;
	}
	else if (faultaddress >= stackbase && faultaddress < stacktop){
		mapped_region = STACK;
	}
	else {
		splx(spl);
		return EFAULT;
	}

	if (as->page_table == NULL)
	{
		as->page_table = pt_create();
		// if no memory available
		if (as->page_table == NULL)
			return ENOMEM;
	}



	int lookup = pt_lookup (as->page_table, faultaddress, faulttype, &entrylo);

	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

	switch (faulttype) {
	    case VM_FAULT_READONLY:
			//We always create pages read-write, so we can't get this 
			//panic("dumbvm: got VM_FAULT_READONLY\n");
			if (lookup==WRITE_PERMITTED)
			{
				// creating a page

				//temp=(vaddr_t)kmalloc(PAGE_SIZE);
				//memcpy(temp,faultaddress & PAGE_FRAME,PAGE_SIZE);
				//update_second_pt_entry(as->page_table, faultaddress, temp, PF_W, &entrylo);
				result = pt_copy_on_write (as->page_table, faultaddress, &entrylo);
				if (result){
					return result;
				}

				int index = TLB_Probe(faultaddress & PAGE_FRAME, 0);
				TLB_Write(faultaddress & PAGE_FRAME, entrylo, index);
				splx(spl);
				return 0;
			}
			else
			{
				panic("dumbvm: got VM_FAULT_READONLY\n");
				// we should kill user thread
				splx(spl);
				return 0;
			}

	    case VM_FAULT_READ:

	    case VM_FAULT_WRITE:

		break;
	    default:
		splx(spl);
		return EINVAL;
	}
	/***************************************************/

	


	if (lookup == PTE_FOUND){
	
		// frame number found update TLB and return
	}

	else if (lookup == NO_SECOND_LEVEL_PT || lookup == INVALID){

		/// 1) Allocate a second level PT
		/// 2) Update first_pt_entry correponding to faultaddress
		/// 3) Allocate a frame for user
		/// 4) Read from disk to user frame if mapped region is CODE or DATA
		/// 5) update the second level pt corresponding to faultaddr
		/// 6) Update TLB for fault addr

		
		if (lookup == NO_SECOND_LEVEL_PT){
			/// 1)
			second_pt =(u_int32_t *) kmalloc(PAGE_SIZE);
			if (second_pt == NULL){
				return ENOMEM;
			}
			bzero((void *) second_pt, PAGE_SIZE);

			/// 2)
			update_first_pt_entry(as->page_table, faultaddress, (vaddr_t) second_pt);
		}
				
		if (mapped_region == CODE || mapped_region == DATA ){
					
			result = as_load_page (faultaddress, mapped_region, &newframe);
			if (result){
				return result;
			}
			flags = as->as_region[mapped_region].p_flags;
		}

		if (mapped_region == HEAP){


			newframe = (vaddr_t)kmalloc(PAGE_SIZE);
			if (! newframe){
				return ENOMEM;
			}
			bzero((void *)newframe, PAGE_SIZE);
			flags = (flags | PF_W);
			}
		
		if (mapped_region == STACK){


			newframe = (vaddr_t)kmalloc(PAGE_SIZE);
			if (! newframe){
				return ENOMEM;
			}
			bzero((void *)newframe, PAGE_SIZE);
			as->as_stackvbase = (faultaddress & PAGE_FRAME);
			flags = (flags | PF_W);
			/***** Handeling  Arguments *****/
			if (curthread->args != NULL)
			{
				
				//kprintf("my tid in stack is %d")
				int j=0;
				int sum=0;
				vaddr_t pointers[16];

				temp= newframe +PAGE_SIZE; // stacktop
		
				for (i=0;i<curthread->argc;i++) // total number of characters
				{
					sum+=strlen(curthread->args[i])+1;
				}
  				temp-=(curthread->argc+1)* sizeof (vaddr_t)+sum;
  				
				sum=0;

  				for(j=0;j<curthread->argc && curthread->args[j]!=NULL ;j++)
  				{
  
	  				memcpy( (void*) temp+sum,(void *) curthread->args[j], strlen(curthread->args[j])+1);
	  				pointers[j]=(faultaddress& PAGE_FRAME)+(temp+sum-newframe);
					sum+=strlen(curthread->args[j])+1;
  				}

				pointers[j]=0;

  				memcpy( temp+sum, pointers, (curthread->argc+1)*sizeof (vaddr_t));

				//kfree(curthread->args);
				curthread->args=NULL;
			}

		}

		update_second_pt_entry(as->page_table, faultaddress, newframe, flags, &entrylo);
	}
	

	else{ /// SWAPPED
		;
	} 
			
			
			
		
	
	/********************UPDATING TLB******************************/

	/* make sure it's page-aligned */
	//assert((paddr & PAGE_FRAME)==paddr);
update_TLB:

	for (i=0; i<NUM_TLB; i++) {
		TLB_Read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress & PAGE_FRAME;
		elo = entrylo;
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		TLB_Write(ehi, elo, i);
		splx(spl);
		return 0;
	}

	//kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
	int rand_entery=random()%64;
	ehi = faultaddress & PAGE_FRAME;
	elo = entrylo;
	DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
	TLB_Write(ehi, elo, rand_entery);
	splx(spl);
	return 0;
}


struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}
	as->as_segnum = 0;
	//as->as_vbase = NULL;
	as->as_vbase1 = 0;
	as->as_pbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_pbase2 = 0;
	as->as_npages2 = 0;
	as->as_region = NULL;
	as->as_stackvbase = 0;
	as->as_heapvtop = 0;
	as->as_stackpbase = 0;
	as->page_table = NULL;	

	return as;
}

///int i=0;
void
as_destroy(struct addrspace *as)
{
	//static int i = 0;
	int spl = splhigh();
	//kprintf("entering as_detroy\n"); 
//	kprintf("as_destroy as, i = %d\n",i);
//	i++;
	if (as->as_pbase1 != 0){
	//	kprintf ("1. 0x%x *** %d\n", as->as_pbase1,i);
		//kfree(PADDR_TO_KVADDR(as->as_pbase1));
	}

	if (as->as_pbase2 != 0){
	//	kprintf ("2. 0x%x  *** %d\n", as->as_pbase2, i);
		//kfree(PADDR_TO_KVADDR(as->as_pbase2));
	}

	if (as->as_stackpbase != 0){
	//	kprintf ("3. 0x%x *** %d\n", as->as_stackpbase, i);
		//kfree(PADDR_TO_KVADDR(as->as_stackpbase));
	}
	//kprintf ("4. 0x%x\n ***\n", as);
	//kfree(as);
	//kprintf("exiting as_detroy\n"); 
	//while(1);
	if (as->page_table)
		pt_deep_destroy(as->page_table);

	if (as->v)
		VOP_DECREF(as->v);

	splx(spl);
}

void
as_activate(struct addrspace *as)
{
	int i, spl;

	(void)as;

	spl = splhigh();

	for (i=0; i<NUM_TLB; i++) {
		TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

void
as_make_readonly()
{
	int i, spl;
	u_int32_t ehi, elo;

	spl = splhigh();

	for (i=0; i<NUM_TLB; i++)
	{
		TLB_Read(&ehi, &elo, i);
		if (elo & TLBLO_VALID)
		{
			elo = elo & (~TLBLO_DIRTY);
			TLB_Write(ehi, elo, i);
		}
	}

	splx(spl);
}

int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
	/// Never gets called region set up is done completley in load_elf
	/// Get rid of it later

	size_t npages; 

	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = sz / PAGE_SIZE;

	/* We don't use these - all pages are read-write */
	(void)readable;
	(void)writeable;
	(void)executable;

	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
		return 0;
	}

	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
		return 0;
	}

	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
}

int
as_prepare_load(struct addrspace *as)
{
	assert(as->as_pbase1 == 0);
	assert(as->as_pbase2 == 0);
	assert(as->as_stackpbase == 0);

	as->as_pbase1 = getppages(as->as_npages1);
	if (as->as_pbase1 == 0) {
		return ENOMEM;
	}

	as->as_pbase2 = getppages(as->as_npages2);
	if (as->as_pbase2 == 0) {
		return ENOMEM;
	}

	as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
	if (as->as_stackpbase == 0) {
		return ENOMEM;
	}

	return 0;
}

int
as_load_page (vaddr_t faultaddress, int mapped_region, vaddr_t *newframe)
{
	vaddr_t startaddr;
	vaddr_t endaddr;
	struct addrspace *as;
	struct uio u;
	u_int32_t offset;
	size_t len;		// amount to actually read
	size_t fillamt;
	int result;
	struct vnode * v;

	// 1) allocate a frame for user
	void * new_frame = kmalloc(PAGE_SIZE);
	if (new_frame == NULL){
		return ENOMEM;
	}
	bzero((void *)new_frame, PAGE_SIZE);

	as = curthread->t_vmspace;
	v = as->v;

	faultaddress &= PAGE_FRAME;
	startaddr = as->as_region[mapped_region].p_vaddr;
	endaddr = as->as_region[mapped_region].p_vaddr + as->as_region[mapped_region].p_filesz;

	offset = as->as_region[mapped_region].p_offset + (faultaddress - startaddr);

	if (faultaddress + PAGE_SIZE <= endaddr){
		len = PAGE_SIZE; 	// amount to actually read
	}
	else {
		if (endaddr > faultaddress)
			len = endaddr-faultaddress;
		else
			len = 0;
	}

	mk_kuio(&u, new_frame, PAGE_SIZE, offset, UIO_READ);
	u.uio_resid = len;

	//kprintf("reading file and vnode\n");
	result = VOP_READ(v, &u);
	if (result) {
		return result;
	}

	if (u.uio_resid != 0) {
			/* short read; problem with executable? */
			kprintf("ELF: short read on phdr - file truncated?\n");
			return ENOEXEC;
	}

	/* Fill the rest of the memory space (if any) with zeros */
	fillamt = PAGE_SIZE - len;
	if (fillamt > 0) {
		DEBUG(DB_EXEC, "ELF: Zero-filling %lu more bytes\n", 
		      (unsigned long) fillamt);
		u.uio_resid += fillamt;
		result = uiomovezeros(fillamt, &u);
	}
	
	*newframe = (vaddr_t)new_frame;
	return result;
}


int
as_complete_load(struct addrspace *as)
{
	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	//assert(as->as_stackpbase != 0);

	*stackptr = USERSTACK;
	return 0;
}

int
as_define_heap(struct addrspace *as, vaddr_t *heapptr)
{
	
	vaddr_t heapbase= as->as_region[1].p_vaddr + as->as_region[1].p_memsz;

	if(heapbase & ~PAGE_FRAME)
	{	
		*heapptr = (heapbase & PAGE_FRAME) + PAGE_SIZE ;
	}
	else
		*heapptr=heapbase;
		as->breakaddr=*heapptr;

	return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace * new;
	int result;

	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}
	

	new->as_segnum = old->as_segnum;

	new->as_region = (struct region *)kmalloc(new->as_segnum * sizeof(struct region));
	if (new->as_region == NULL){
		return ENOMEM;
	}


	//Copy all parents regions info into the child
	int i =0 ;
	for (i = 0; i < new->as_segnum; i++){
		new->as_region[i].p_vaddr = old->as_region[i].p_vaddr;
		new->as_region[i].p_offset = old->as_region[i].p_offset;
		new->as_region[i].p_filesz = old->as_region[i].p_filesz;
		new->as_region[i].p_memsz = old->as_region[i].p_memsz;
		new->as_region[i].p_flags = old->as_region[i].p_flags;
	} 

	new->as_stackvbase = old->as_stackvbase;
	new->as_heapvtop = old->as_heapvtop;

	//both parent and child are pointing to the same file intially
	
	new->v = old->v;
	VOP_INCREF(new->v);


    // Now we need to to make a copy of parents page table for child
    // Since we might run out of memory at any point we have to make sure
    // change the parent page table (making it read only)once we are sure
    // that child page tables are completely set up and is ready to run.  
	// We copy them here

	result = pt_copy(old->page_table, & (new->page_table));
	if (result){
		VOP_DECREF(new->v); // Decrement refrence count by 1. when reference count is 0, file close
		// Destroy address space
		// Destroy new page tables
		if (new->page_table)
			pt_destroy(new->page_table);
	}
		
	
	*ret = new;
	return 0;
}
