/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>
 #include <queue.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	//kprintf("locked %s\n",sem->name);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	//kprintf("unlocked %s\n",sem->name);
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	//kprintf ("@@@@@@@@@****the address of name given is 0x%x  ***\n", lock->name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	lock->busy = 0;
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	int spl;
	
	spl = splhigh();
	assert(thread_hassleepers(lock)==0);
	splx(spl);

	//kprintf ("****the address of name is 0x%x  ***\n", lock->name);
	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	int spl;
	assert(lock  != NULL);

    // disabling interrupts
	spl = splhigh();
	while (lock->busy != 0){
		thread_sleep(lock);
	}
	/* Assigning current thread id to lock variable so we can later determine
	 * if some locks are holding by the current thread or not 
	 */  
	lock->busy = curthread;
	splx(spl);
}

void
lock_release(struct lock *lock)
{
	int spl;
	assert(lock != NULL);

	spl = splhigh();
	// Freeing the lock
	lock->busy = 0;		
	/* 
	 * Sending all threads waiting for the lock to the run queue
	 * so scheduler will able can dispatch them later. 
	 */
	thread_wakeup(lock);
	splx(spl);
}

int
lock_do_i_hold(struct lock *lock)
{
	assert(lock != NULL);
	//int spl = splhigh();
	if (lock->busy == curthread)
		return 1;
	//splx(spl);

	return 0;
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	cv->slept_threads = q_create(10);
	if (cv->slept_threads == NULL) {
		return NULL;
	}
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);
    
    //int spl = splhigh();

	q_destroy(cv->slept_threads);
	
	kfree(cv->name);
	kfree(cv);
	//splx(spl);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	/*Interrupt should be disabled before calling cv_wait*/
	assert(cv != NULL && lock != NULL);
	int spl = splhigh();

	q_addtail(cv->slept_threads, curthread);  //push the address of current thread id into corresponding cv_queue  
	lock_release(lock);		//releasing lock
	thread_sleep(curthread);		//thread sleeps on its own address

	splx(spl);
	lock_acquire (lock);	
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	/* Wake up one thread that's sleeping on this CV. */
	assert(cv != NULL && lock != NULL);
	int spl = splhigh();
	void* addr = q_remhead(cv->slept_threads);
	thread_wakeup(addr);
	splx(spl);
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	/*Wake up all threads sleeping on this CV.*/
	assert(cv != NULL && lock != NULL);
	int spl = splhigh();
	void* addr;
	while (!q_empty(cv->slept_threads)) {
		addr = q_remhead(cv->slept_threads);
		thread_wakeup(addr);
	}
	splx(spl);
}
