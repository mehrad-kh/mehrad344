/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
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
 * Number of cars created.
 */
#define NCARS 20
/*
 *
 * Function Definitions
 *
 */
static const char *directions[] = { "N", "E", "S", "W" };


enum { N_W, N_E , S_E, S_W };


static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};
/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };
struct intersection{
    /*
    * pointer to four binary semaphores for locking 
    * each quadrant of the intersection 
    */
	struct semaphore * NE;
	struct semaphore * NW;
	struct semaphore * SE;
	struct semaphore * SW;
	struct semaphore * staus;

    /*
    * pointer to array queues of size 4
    * each queue keeps track of order of 
    * of cars approaching its corresponding
    * entry of the intersection 
    */
	struct queue ** waitlist;

    /*
    * binary semaphore to provide mutual exclusion
    * for each queue. 
    */
    struct semaphore ** waitlist_lock;


	int carnumber;   // numbaer of cars in the intersection at any point of time
	int carleaving;  // for debugging purposes
};
static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}


/*
* determines whether it is the carnumber's turn to enter the  
* intersection or not. If so, return 1, else return 0. 
*/
static int
is_in_order (struct queue * waitlist, void *carnumber)
{
    // Check whether the queue is empty
    if (q_empty(waitlist))
    {
        return 1;
    }
    // check whether the carnumber has the permission to enter the region
    if (q_getguy(waitlist, q_getstart(waitlist)) == carnumber)
    {
        return 1; 
    }
    return 0;
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      APPROACHINGunsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */
static
void
gostraight(void * INTERSECTION,unsigned long cardirection,
           unsigned long carnumber)
{
	struct intersection * intersection= INTERSECTION;
	int region=0;
        
	P(intersection->staus);
	q_addtail(intersection->waitlist[cardirection], &carnumber);
	message(APPROACHING, carnumber, cardirection,(cardirection+2)%4);
	V(intersection->staus);

	while( region<2)
	{
		if(cardirection==0)
		{
			if (region==0)
			{
				P(intersection->NW);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[N_W], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->NW);
					
				}
					
				else{
					q_remhead(intersection->waitlist[N_W]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 2);
				}
			}
			else if (region==1)
			{
				P(intersection->SW);
				message(REGION2, carnumber, cardirection, 2);
				V(intersection->NW);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 2);
				V(intersection->SW);
			}
		}
		else if(cardirection==1)
		{
			if (region==0)
			{
				P(intersection->NE);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[N_E], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->NE);
					
				}
					
				else{
					q_remhead(intersection->waitlist[N_E]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 3);
				}
			}
			else if (region==1)
			{
				P(intersection->NW);
				message(REGION2, carnumber, cardirection, 3);
				V(intersection->NE);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 3);
				V(intersection->NW);
			}
			
		}
		else if(cardirection==2)
		{
			if (region==0)
			{
				P(intersection->SE);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[S_E], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->SE);
					
				}
					
				else{
					q_remhead(intersection->waitlist[S_E]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 0);
				}
			}
			else if (region==1)
			{
				P(intersection->NE);
				message(REGION2, carnumber, cardirection, 0);
				V(intersection->SE);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection,0);
				V(intersection->NE);
			}
			
		}
		else if(cardirection==3)
		{
			if (region==0)
			{
				P(intersection->SW);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[S_W], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->SW);
					
				}
					
				else
				{
					q_remhead(intersection->waitlist[S_W]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 1);
				}
			
			}
			else if (region==1)
			{
				P(intersection->SE);
				message(REGION2, carnumber, cardirection, 1);
				V(intersection->SW);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 1);
				V(intersection->SE);
			}
		}
	} 
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */
static
void
turnleft(void * INTERSECTION, unsigned long cardirection,
         unsigned long carnumber)
{
	struct intersection * intersection= INTERSECTION;   
	int region=0;

	P(intersection->staus);
	q_addtail(intersection->waitlist[cardirection], &carnumber);
	message(APPROACHING, carnumber, cardirection,(cardirection+1)%4);
	V(intersection->staus);

	while( region<3)
	{
		if(cardirection==0)
		{
			if (region==0)
			{	
				P(intersection->NW);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[N_W], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->NW);
					
				}
					
				else{
					q_remhead(intersection->waitlist[N_W]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 1);
				}
			}
			else if (region==1)
			{
				P(intersection->SW);
				message(REGION2, carnumber, cardirection, 1);
				V(intersection->NW);
				region++;
			}
			else
			{
				P(intersection->SE);
				message(REGION3, carnumber, cardirection, 1);
				V(intersection->SW);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 1);
				
				V(intersection->SE);		
			}
		}
		else if(cardirection==1)
		{
			if (region==0)
			{
				P(intersection->NE);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[N_E], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->NE);
					
				}
					
				else{
					q_remhead(intersection->waitlist[N_E]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 2);
				}
			}
			else if (region==1)
			{
				P(intersection->NW);
				message(REGION2, carnumber, cardirection, 2);
				V(intersection->NE);
				region++;
			}
			else
			{
				P(intersection->SW);
				message(REGION3, carnumber, cardirection, 2);
				V(intersection->NW);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 2);
				
				V(intersection->SW);		
			}
		}
		else if(cardirection==2)
		{
			if (region==0)
			{
				P(intersection->SE);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[S_E], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->SE);
					
				}
					
				else{
					q_remhead(intersection->waitlist[S_E]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 3);
				}
				
			}
			else if (region==1)
			{
				P(intersection->NE);
				message(REGION2, carnumber, cardirection, 3);
				V(intersection->SE);
				region++;	
			}
			else
			{
				P(intersection->NW);
				message(REGION3, carnumber, cardirection, 3);
				V(intersection->NE);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 3);
				
				V(intersection->NW);	
			}
		}
		else if(cardirection==3)
		{
			if (region==0)
			{
				P(intersection->SW);
				P(intersection->staus);
				if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[S_W], &carnumber)  )
				{	
					
					V(intersection->staus);
					V(intersection->SW);
					
				}
					
				else{
					q_remhead(intersection->waitlist[S_W]);
					intersection->carnumber++;
					V(intersection->staus);
					region++;
					message(REGION1, carnumber, cardirection, 0);
				}
			}
			else if (region==1)
			{
				P(intersection->SE);
				message(REGION2, carnumber, cardirection, 0);
				V(intersection->SW);
				region++;
			}
			else
			{
				P(intersection->NE);
				message(REGION3, carnumber, cardirection, 0);
				V(intersection->SE);
				region++;
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection,0);
				
				V(intersection->NE);	
			}
		}
	}       
}



/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */
static
void
turnright(void * INTERSECTION, unsigned long cardirection,
          unsigned long carnumber)
{
    struct intersection * intersection= INTERSECTION;
	int done=0;

	P(intersection->staus);
	q_addtail(intersection->waitlist[cardirection], &carnumber);
	message(APPROACHING, carnumber, cardirection,(cardirection+3)%4);
	V(intersection->staus);

	while(!done)
	{
		if(cardirection==0)
		{
			P(intersection->NW);
			P(intersection->staus);
			if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[N_W], &carnumber)  )
			{	
					
				V(intersection->staus);
				V(intersection->NW);
					
			}
					
			else{
				q_remhead(intersection->waitlist[N_W]);
				intersection->carnumber++;
				V(intersection->staus);
				message(REGION1, carnumber, cardirection, 3);
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 3);
				done=1;
				
				V(intersection->NW);
			}
				
		}
		else if(cardirection==1)
		{
			P(intersection->NE);
			P(intersection->staus);
			if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[N_E], &carnumber)  )
			{	
				
				V(intersection->staus);
				V(intersection->NE);
				
			}
				
			else{
				q_remhead(intersection->waitlist[N_E]);
				intersection->carnumber++;
				V(intersection->staus);
				message(REGION1, carnumber, cardirection, 0);
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 0);
				done=1;
				
				V(intersection->NE);
			}
		
		}
		else if(cardirection==2)
		{
			P(intersection->SE);
			P(intersection->staus);
			if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[S_E], &carnumber)  )
			{	
				
				V(intersection->staus);
				V(intersection->SE);
				
			}
				
			else{
				q_remhead(intersection->waitlist[S_E]);
				intersection->carnumber++;
				V(intersection->staus);
				message(REGION1, carnumber, cardirection, 1);
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection,1);
				done=1;
				
				V(intersection->SE);
			}
		}
		else if(cardirection==3)
		{
			P(intersection->SW);
			P(intersection->staus);
			if (intersection->carnumber>2 || !is_in_order (intersection->waitlist[S_W], &carnumber)  )
			{	
				
				V(intersection->staus);
				V(intersection->SW);
				
			}
				
			else{
				q_remhead(intersection->waitlist[S_W]);
				intersection->carnumber++;
				V(intersection->staus);
				message(REGION1, carnumber, cardirection, 2);
				P(intersection->staus);
				intersection->carnumber--;
				intersection->carleaving++;
				V(intersection->staus);
				message(LEAVING, carnumber, cardirection, 2);
				done=1;
				V(intersection->SW);
			}
		}
	}      
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this fungostraightction as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * INTERSECTION,
                     unsigned long carnumber)
{
        int cardirection;
	int destination;
      
    /*
    * cardirection is set randomly.
    */
    cardirection = random() % 4;
	destination=random() % 3;
	if (destination==0)
	{
	
		turnright(INTERSECTION,cardirection,carnumber);
		
	}
	else if (destination==1)
	{
		
		turnleft(INTERSECTION,cardirection,carnumber);
		
	}
	else if (destination==2)
	{
		
		gostraight(INTERSECTION,cardirection,carnumber);
		
	}
	
}
/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */
int
createcars(int nargs,
           char ** args)
{
        int index, error;
	struct intersection * INTERSECTION= (struct intersection *) kmalloc(sizeof(struct intersection )) ;
        /*
         * Avoid unused variable warnings.
         */
        (void) nargs;
        (void) args;
	// initialization
	
	INTERSECTION->NE=sem_create("NE",1);
	INTERSECTION->NW=sem_create("NW",1);
	INTERSECTION->SE=sem_create("SE",1);
	INTERSECTION->SW=sem_create("SW",1);
	INTERSECTION->staus=sem_create("status",1);
	INTERSECTION->carnumber=0;
	INTERSECTION->carleaving=0;
   	INTERSECTION->waitlist= (struct queue **)kmalloc (4*sizeof(struct queue *));
    INTERSECTION->waitlist_lock = (struct semaphore **)kmalloc(4*sizeof(struct semaphore *));
    
    INTERSECTION->waitlist_lock[N_W] = sem_create("NW_waitlist", 1);
    INTERSECTION->waitlist_lock[N_E] = sem_create("NE_waitlist", 1);
    INTERSECTION->waitlist_lock[S_E] = sem_create("SE_waitlist", 1);
    INTERSECTION->waitlist_lock[S_W] = sem_create("SW_waitlist", 1);
    
	int i = 0;
    	for (i = 0; i < 4; i++){
        	INTERSECTION->waitlist[i] = q_create(NCARS);
    	}
        /*
         * Start NCARS approachintersection() threads.
         */
        for (index = 0; index < NCARS; index++) {
                error = thread_fork("approachintersection thread",
                                    INTERSECTION,
                                    index,
                                    approachintersection,
                                    NULL
                                    );
                /*
                 * panic() on error.
                 */
                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }
        return 0;
}
