 /*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2


struct HOUSE {
    int busycat;
    int busymouse;
    int bowls[NFOODBOWLS];
    struct lock* bowls_lock;
    //struct lock* kprintf_lock;
    struct cv* cat_spot;
    struct cv* mouse_spot;
};
/*
 * 
 * Function Definitions
 * 
 */

/* who should be "cat" or "mouse" */
static void
lock_eat(const char *who, int num, int bowl, int iteration)
{
        //lock_acquire(kprintf_lock);
        kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
        //lock_release(kprintf_lock);
        clocksleep(1);
        //lock_acquire(kprintf_lock);
        kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
       // lock_release(kprintf_lock);
}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(struct HOUSE* house, 
        unsigned long catnumber)
{
    int iteration = 0;
    while (iteration <= 3)
    {
        lock_acquire(house->bowls_lock); // locking for mutal exclusion related to house object
        while (house->busymouse > 0 || house->busycat == NFOODBOWLS ) // Checking if a mouse is present or bowls are full
            cv_wait(house->cat_spot, house->bowls_lock); // letting other threads run, avvoiding spening and slowing down the code
  
        int s;
        int flag = 0;
        for (s = 0; s < NFOODBOWLS && flag == 0; s++) // checking which bowl is empty
        {
            if (house->bowls[s] == 0) 
            {
                flag = 1;
                house->bowls[s] = 1;
                house->busycat++;
                lock_release(house->bowls_lock); // unlocking letting more than two cats eat at the same time
                lock_eat("cat", catnumber, s+1, iteration);
                lock_acquire(house->bowls_lock); // locking for mutal exclusion
                iteration++;
                house->busycat--;
                house->bowls[s] = 0;

             
                cv_broadcast(house->cat_spot, house->bowls_lock); // tells to all cats to wake up
                if (house->busycat == 0) // incase no cat is eating wake up the mice
                    cv_broadcast(house->mouse_spot, house->bowls_lock);

                lock_release(house->bowls_lock); // letting the lock go
            }
        }
    }
}
    

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(struct HOUSE* house,
          unsigned long mousenumber)
{
int iteration = 0;

while(iteration <= 3)
{
	
	lock_acquire(house->bowls_lock); // locking for mutal exclusion related to house object
	while (house->busycat > 0 || house->busymouse == NFOODBOWLS ) // Checking if a cat is present or bowls are full
	    cv_wait(house->mouse_spot, house->bowls_lock); // letting other threads run, avvoiding spening and slowing down the code

	int s;
	int flag = 0; 

	for (s = 0; s < NFOODBOWLS && flag == 0; s++) // checking which bowl is empty
	{
	    if (house->bowls[s] == 0) 
	    {
		flag = 1;
		house->bowls[s] = 1;
		house->busymouse++;
		lock_release(house->bowls_lock);  // unlocking letting more than two mice eat at the same time
		lock_eat("mouse", mousenumber, s+1, iteration);
		lock_acquire(house->bowls_lock); // locking for mutal exclusion
		iteration++;
		house->busymouse--;
		house->bowls[s] = 0;

		cv_broadcast(house->mouse_spot, house->bowls_lock);  // tells to all the mice to wake up
		if (house->busymouse == 0)
		    cv_broadcast(house->cat_spot, house->bowls_lock); // incase no cat is eating wake up the cats

		lock_release(house->bowls_lock);
	    }
	}
}
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;


        /* Initializing Shared Data */

        struct HOUSE * house = (struct HOUSE *) kmalloc (sizeof(struct HOUSE));
        house->busycat = 0; // stores number of cat eating at a give time
        house->busymouse = 0; // stores number of mice eating at a give ntime
        int i;
        for (i = 0; i < NFOODBOWLS; i++) // data realated to the bowls
            house->bowls[i] = 0;
        house->bowls_lock = lock_create("bowls lock"); // the lock
        house->cat_spot = cv_create("cat spot");
        house->mouse_spot = cv_create("mouse spot");
        

        /*
         * Start NCATS catlock() threads.
         */

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    house, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    house, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        return 0;
}

/*
 * End of catlock.c
 */
