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

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

enum { NE, SE, SW, NW };

struct intersection{
	struct semaphore * NE;
	struct semaphore * NW;
	struct semaphore * SE;
	struct semaphore * SW;
    struct semaphore * lock_status;
    struct queue ** waitlist_quad;
    struct semaphore ** waitlist_lock;

    /*
    * array of size four. Indecies are associated to 
    * NE, SE, SW and NW quadrant respectively and their 
    * elements corresponds to the cars' next step direction  
    * in that quadrant. If there is no car in a quadrant its 
    * direction is -1.
    */
    int * status;

};


/* ***Deadlock Condition***
* Deadlock happen if there are four cars in the intersection 
* and none of them are to move to outward direction away from
* the intersection in the next step. 
*/
static int
is_deadlock (int *status, int size, int dep_quadrant, int des_quadrant) 
{
    int i = 0;

    int temp = status[dep_quadrant];
    status[dep_quadrant] = des_quadrant;

    kprintf("looping here\n");
    for (i = 0; i < size; i++){
        if (status[i] == -1 || status [i] == i)
        {
            status[dep_quadrant] = temp;
            return 0;
        }
    }
    status[dep_quadrant] = temp;
    return 1;

}

static void 
update_status (int* status, int quadrant, int destination, struct semaphore * lock_status)
{
    P(lock_status);
    status[quadrant] = destination;
    V(lock_status);
}


static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}


static void
atomic_enqueue(struct queue * waitlist, struct semaphore * waitlist_lock, void* carnumber, int cardirection, int destdirection)
{
    P(waitlist_lock);
    q_addtail(waitlist, carnumber);
    message(APPROACHING, *(int *)carnumber, cardirection, destdirection);
    V(waitlist_lock);
    
}

static void
atomic_dequeue(struct queue * waitlist, struct semaphore * waitlist_lock)
{
    P(waitlist_lock);
    void * ret = q_remhead(waitlist);
    kprintf("car: %d entered the intersection and removed from waitlist\n",*(int *)ret);
    V(waitlist_lock);
}

static int
is_in_order (struct queue * waitlist, struct semaphore * waitlist_lock, void *carnumber)
{
    P(waitlist_lock);

    if (q_empty(waitlist))
    {
        V(waitlist_lock);
        return 1;
    }
    if (get_head(waitlist) == carnumber)
    {
        V(waitlist_lock);
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
        /*
         * Avoid unused variable warnings.
         */
	int region = 0;
    int done = 0;

    //message(APPROACHING, carnumber, cardirection, (cardirection+2)%4);

	while( region<2)
	{
		if(cardirection==0)   // north to south approaching NW quadrant
		{
            //message(APPROACHING, carnumber, cardirection, 2);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[NW], intersection->waitlist_lock[NW], (void*)&carnumber, cardirection, 2);
                //P(intersection->NW);
                //message(APPROACHING, carnumber, cardirection, 2);
                while (!done)
                {
                    P(intersection->NW);
                    P(intersection->lock_status);

                    if (! is_deadlock(intersection->status, 4, NW, SW) && is_in_order (intersection->waitlist_quad[NW], intersection->waitlist_lock[NW], &carnumber))
				    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 2);
                        update_status (intersection->status, NW, SW, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[NW], intersection->waitlist_lock[NW]);
                        done = 1;
                       // V(intersection->NW);
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->NW);  
                    }   
                }
                //V(intersection->NW);
			}
			else if (region==1) 
			{
                // No need to check for deadlock. Moving away from intersection
				P(intersection->SW);
				region++;
                update_status (intersection->status, SW, -1, intersection->lock_status);
				message(REGION2, carnumber, cardirection, 2);
                update_status (intersection->status, NW, -1, intersection->lock_status);
                V(intersection->NW);
                message(LEAVING, carnumber, cardirection, 2);
                V(intersection->SW);
			}
		}



		else if(cardirection==1)  // east to west approaching NE quadrant
		{
            //message(APPROACHING, carnumber, cardirection, 3);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[NE], intersection->waitlist_lock[NE], &carnumber, cardirection, 3);
                //P(intersection->NE);
                //message(APPROACHING, carnumber, cardirection, 3);
                while(!done)
                {
				    P(intersection->NE);
                    P(intersection->lock_status);
                    if (! is_deadlock(intersection->status, 4, NE, NW) && is_in_order (intersection->waitlist_quad[NE], intersection->waitlist_lock[NE], &carnumber))
                    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 3);
                        update_status (intersection->status, NE, NW, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[NE], intersection->waitlist_lock[NE]);
                        done = 1;
                        //V(intersection->NE);
                    }
                    else
                    {
                        V(intersection->lock_status);  
                        V(intersection->NE);
                    }	    
                }
                //V(intersection->NE);
			}
			else if (region==1)
			{
                // No need to check for deadlock. Moving away from intersection
				P(intersection->NW);
				region++;
                update_status (intersection->status, NE, -1, intersection->lock_status);
				message(REGION2, carnumber, cardirection, 3);
                update_status (intersection->status, NW, -1, intersection->lock_status);
                V(intersection->NE);
				message(LEAVING, carnumber, cardirection, 3);
                V(intersection->NW);
			}			
		}



		else if(cardirection==2)  // south to north approaching SE quadrant
		{
            //message(APPROACHING, carnumber, cardirection, 0);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[SE], intersection->waitlist_lock[SE], &carnumber, cardirection, 0);
                //P(intersection->SE);
                //message(APPROACHING, carnumber, cardirection, 0);
                while (!done)
                {
				    P(intersection->SE);
                    P(intersection->lock_status);
                    if (! is_deadlock(intersection->status, 4, SE, NE) && is_in_order (intersection->waitlist_quad[SE], intersection->waitlist_lock[SE], &carnumber))
                    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 0);
                        update_status (intersection->status, SE, NE, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[SE], intersection->waitlist_lock[SE]);
                        done = 1;
                        //V(intersection->SE);
                    }
                    else
                    {
                        V(intersection->lock_status); 
                        V(intersection->SE);
                    } 
                }
                //V(intersection->SE);
			}
			else if (region==1)
			{
                // No need to check for deadlock. Moving away from intersection
				P(intersection->NE);
				region++;
                update_status (intersection->status, SE, -1, intersection->lock_status);
				message(REGION2, carnumber, cardirection, 0);
                update_status (intersection->status, NE, -1, intersection->lock_status);
                V(intersection->SE);
				
                message(LEAVING, carnumber, cardirection, 0);
                V(intersection->NE);
			}
			
		}



		else if(cardirection==3)     // west to east approaching SW quadrant
		{
           // message(APPROACHING, carnumber, cardirection, 1);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[SW], intersection->waitlist_lock[SW], &carnumber, cardirection, 1);
                //P(intersection->SW);
                //message(APPROACHING, carnumber, cardirection, 1);
                while (!done)
                {
                    P(intersection->SW);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, SW, SE) && is_in_order (intersection->waitlist_quad[SW], intersection->waitlist_lock[SW], &carnumber))
                    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 1);
                        update_status (intersection->status, SW, SE, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[SW], intersection->waitlist_lock[SW]);
                        done = 1;
                        //V(intersection->SW);
                    }

                    else 
                    {
                        V(intersection->lock_status);
                        V(intersection->SW);
                    }  
                }
                //V(intersection->SW);
			}
			else if (region==1)
			{
                // No need to check for deadlock. Moving away from intersection
				P(intersection->SE);
				region++;
                update_status (intersection->status, SW, -1, intersection->lock_status);
				message(REGION2, carnumber, cardirection, 1);
                update_status (intersection->status, SE, -1, intersection->lock_status);
                V(intersection->SW);
				
                message(LEAVING, carnumber, cardirection, 1);
                V(intersection->SE);
			}
		}
	}

    //message(LEAVING, carnumber, cardirection, (cardirection+2)%4);           
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
        /*
         * Avoid unused variable warnings.
         */
	int region = 0;
    int done_1 = 0;
    int done_2 = 0;

    //message(APPROACHING, carnumber, cardirection, (cardirection+1)%4);

	while( region<3)
	{
		if(cardirection==0)   // north to east approaching NW
		{
            //message(APPROACHING, carnumber, cardirection, 1);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[NW], intersection->waitlist_lock[NW], &carnumber, cardirection, 1);
                //P(intersection->NW);
                //message(APPROACHING, carnumber, cardirection, 1);
                while (!done_1)
                {
                    P(intersection->NW);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, NW, SW) && is_in_order (intersection->waitlist_quad[NW], intersection->waitlist_lock[NW], &carnumber))
                    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 1);
                        update_status (intersection->status, NW, SW, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[NW], intersection->waitlist_lock[NW]);
                        done_1 = 1;
                        //V(intersection->NW);
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->NW); 
                    }
                    
                }
                //V(intersection->NW);
			}
			else if (region==1)
			{
                while (!done_2)
                {
                    P(intersection->SW);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, SW, SE))
                    {
                        V(intersection->lock_status);
                        region++;
                        update_status (intersection->status, NW, -1, intersection->lock_status);
                        message(REGION2, carnumber, cardirection, 1);
                        update_status (intersection->status, SW, SE, intersection->lock_status);
                        V(intersection->NW);
                        done_2 = 1;
                        
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->SW); 
                    }
                    
                }
				
			}
			else
			{
                // No need to check for deadlock. Moving away from intersection
				P(intersection->SE);
				region++;
                update_status (intersection->status, SW, -1, intersection->lock_status);
				message(REGION3, carnumber, cardirection, 1);
                update_status (intersection->status, SE, -1, intersection->lock_status);
                V(intersection->SW);
                message(LEAVING, carnumber, cardirection, 1);	
                V(intersection->SE);
			}
		}



		else if(cardirection==1)  // east to south approaching NE
		{
            //message(APPROACHING, carnumber, cardirection, 2);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[NE], intersection->waitlist_lock[NE], &carnumber, cardirection, 2);
                //P(intersection->NE);
                //message(APPROACHING, carnumber, cardirection, 2);
                while (!done_1)
                {
                    //P(intersection->NE);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, NE, NW) && is_in_order (intersection->waitlist_quad[NE], intersection->waitlist_lock[NE], &carnumber))
                    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 2);
                        update_status (intersection->status, NE, NW, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[NE], intersection->waitlist_lock[NE]);
                        done_1 = 1;
                        //V(intersection->NE);
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->NE);
                    }
                     
                }
                //V(intersection->NE);	
			}
			else if (region==1)
			{
                while (!done_2)
                {
                    P(intersection->NW);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, NW, SW))
                    {
                        V(intersection->lock_status);
                        region++;
                        update_status (intersection->status, NE, -1, intersection->lock_status);
                        message(REGION2, carnumber, cardirection, 2);
                        update_status (intersection->status, NW, SW, intersection->lock_status);
                        V(intersection->NE);
                        done_2 = 1;
                        //V(intersection->NW); 
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->NW);
                    }
                    
                }
				
			}
			else
			{
                // No need to check for deadlock. Moving away from intersection
				P(intersection->SW);
				region++;
                update_status (intersection->status, NW, -1, intersection->lock_status);
				message(REGION3, carnumber, cardirection, 2);
                update_status (intersection->status, SW, -1, intersection->lock_status);
                V(intersection->NW); 
					
                message(LEAVING, carnumber, cardirection, 2);
                V(intersection->SW);
			}
		}



		else if(cardirection==2)     // south to west approaching SE
		{
            //message(APPROACHING, carnumber, cardirection, 3);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[SE], intersection->waitlist_lock[SE], &carnumber, cardirection, 3);
                //P(intersection->SE);
                //message(APPROACHING, carnumber, cardirection, 3);
                while (!done_1)
                {
                    P(intersection->SE);
                    P(intersection->lock_status);
                    if(!is_deadlock(intersection->status, 4, SE, NE) && is_in_order (intersection->waitlist_quad[SE], intersection->waitlist_lock[SE], &carnumber))
                    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 3);
                        update_status (intersection->status, SE, NE, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[SE], intersection->waitlist_lock[SE]);
                        done_1 = 1;
                        //V(intersection->SE);
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->SE);
                    }
                }
				//V(intersection->SE);
			}
			else if (region==1)
			{
                while (!done_2)
                {
                    P(intersection->NE);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, NE, NW))
                    {
                        V(intersection->lock_status);
                        region++;
                        update_status (intersection->status, SE, -1, intersection->lock_status);
                        message(REGION2, carnumber, cardirection, 3);
                        update_status (intersection->status, NE, NW, intersection->lock_status);
                        V(intersection->SE);
                        done_2 = 1;
                        //V(intersection->NE);
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->NE);
                     
                    }
                }
				
			}
			else
			{
                // No need to check for deadlock. Moving away from intersection
				P(intersection->NW);
				region++;
                update_status (intersection->status, NE, -1, intersection->lock_status);
				message(REGION3, carnumber, cardirection, 3);
                update_status (intersection->status, NW, -1, intersection->lock_status);
                V(intersection->NE);
					
                message(LEAVING, carnumber, cardirection, 3);
                V(intersection->NW);
			}
		}



		else if(cardirection==3)    // west to north approaching SW
		{
            //message(APPROACHING, carnumber, cardirection, 0);
			if (region==0)
			{
                atomic_enqueue(intersection->waitlist_quad[SW], intersection->waitlist_lock[SW], &carnumber, cardirection, 0);
                //P(intersection->SW);
                //message(APPROACHING, carnumber, cardirection, 0);
                while (!done_1)
                {
                    P(intersection->SW);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, SW, SE) && is_in_order (intersection->waitlist_quad[SW], intersection->waitlist_lock[SW], &carnumber))
                    {
                        V(intersection->lock_status);
                        region++;
                        message(REGION1, carnumber, cardirection, 0);
                        update_status (intersection->status, SW, SE, intersection->lock_status);
                        atomic_dequeue(intersection->waitlist_quad[SW], intersection->waitlist_lock[SW]);
                        done_1 = 1;
                        //V(intersection->SW); 
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->SW);
                    }
                    
                }
				//V(intersection->SW);
			}
			else if (region==1)
			{
                while (!done_2)
                {
                    P(intersection->SE);
                    P(intersection->lock_status);
                    if (!is_deadlock(intersection->status, 4, SE, NE))
                    {
                        V(intersection->lock_status);
                        region++;
                        update_status (intersection->status, SW, -1, intersection->lock_status);
                        message(REGION2, carnumber, cardirection, 0);
                        update_status (intersection->status, SE, NE, intersection->lock_status);
                        V(intersection->SW);
                        done_2 = 1;
                        //V(intersection->SE); 
                    }
                    else
                    {
                        V(intersection->lock_status);
                        V(intersection->SE); 
                    }
                }
				
			}
			else
			{
				P(intersection->NE);
				region++;
                update_status (intersection->status, SE, -1, intersection->lock_status);
				message(REGION3, carnumber, cardirection, 0);
                update_status (intersection->status, NE, -1, intersection->lock_status);
                V(intersection->SE); 
				
                message(LEAVING, carnumber, cardirection, 0);	
                V(intersection->NE);
			}
		}
	}

    //message(LEAVING, carnumber, cardirection, (cardirection+1)%4);  
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
        
        /* 
        * These moves will not create deadlock at all 
        * since their next moves are alaways in outward 
        * direction of intersection. As long as a quadrant
        * is free it is safe to go in.
        */
	//message(APPROACHING, carnumber, cardirection, (cardirection+3)%4);
    int done = 0;

	if(cardirection==0)    // north to west
	{
        //message(APPROACHING, carnumber, cardirection, 3);
        atomic_enqueue(intersection->waitlist_quad[NW], intersection->waitlist_lock[NW], &carnumber, cardirection, 3);
        while (!done)
        {
            P(intersection->NW);
            //message(APPROACHING, carnumber, cardirection, (cardirection+3)%4);
            if (is_in_order (intersection->waitlist_quad[NW], intersection->waitlist_lock[NW], &carnumber))
            {
                message(REGION1, carnumber, cardirection, 3);
                atomic_dequeue(intersection->waitlist_quad[NW], intersection->waitlist_lock[NW]);
                done = 1;
                message(LEAVING, carnumber, cardirection, (cardirection+3)%4);
                V(intersection->NW);
            }
           
            else
            {
                V(intersection->NW);
            }

        }
			
	}
	else if(cardirection==1)   // east
	{
		//message(APPROACHING, carnumber, cardirection, 0);
        atomic_enqueue(intersection->waitlist_quad[NE], intersection->waitlist_lock[NE], &carnumber, cardirection, 0);
        while (!done)
        {
            P(intersection->NE);
            if (is_in_order (intersection->waitlist_quad[NE], intersection->waitlist_lock[NE], &carnumber))
            {
                //message(APPROACHING, carnumber, cardirection, (cardirection+3)%4);
                message(REGION1, carnumber, cardirection, 0);
                atomic_dequeue(intersection->waitlist_quad[NE], intersection->waitlist_lock[NE]);
                done = 1;
                message(LEAVING, carnumber, cardirection, (cardirection+3)%4);
                V(intersection->NE);
            }
          
            else
            {
                V(intersection->NE);
            }
        }
	
		
	}



	else if(cardirection==2)   // south
	{
		//message(APPROACHING, carnumber, cardirection, 1);
        atomic_enqueue(intersection->waitlist_quad[SE], intersection->waitlist_lock[SE], &carnumber, cardirection, 1);
        while (!done)
        {
            P(intersection->SE);
            if (is_in_order (intersection->waitlist_quad[SE], intersection->waitlist_lock[SE], &carnumber))
            {
                //message(APPROACHING, carnumber, cardirection, (cardirection+3)%4);
                message(REGION1, carnumber, cardirection, 1);
                atomic_dequeue(intersection->waitlist_quad[SE], intersection->waitlist_lock[SE]);
                done = 1;
                message(LEAVING, carnumber, cardirection, (cardirection+3)%4);
                V(intersection->SE);
            }
            else
            {
                V(intersection->SE);
            }
        }
		
	}



	else if(cardirection==3)   // west
	{
	    //message(APPROACHING, carnumber, cardirection, 2);
        atomic_enqueue(intersection->waitlist_quad[SW], intersection->waitlist_lock[SW], &carnumber, cardirection, 2);
        while (!done)
        {
            P(intersection->SW);
            if (is_in_order (intersection->waitlist_quad[SW], intersection->waitlist_lock[SW], &carnumber))
            {
                message(REGION1, carnumber, cardirection, 2);
                done = 1;
                atomic_dequeue(intersection->waitlist_quad[SW], intersection->waitlist_lock[SW]);
                message(LEAVING, carnumber, cardirection, (cardirection+3)%4);
                V(intersection->SW);
            }
            else
            {
                V(intersection->SW); 
            }
            
        }
		
        //message(LEAVING, carnumber, cardirection, 2);
	}	
    
    //message(LEAVING, carnumber, cardirection, (cardirection+3)%4);   
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

        cardirection =  random() % 2;
	destination= random() % 3;
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
	
	INTERSECTION->NE = sem_create("NE",1);
	INTERSECTION->NW = sem_create("NW",1);
	INTERSECTION->SE = sem_create("SE",1);
	INTERSECTION->SW = sem_create("SW",1);
    INTERSECTION->lock_status = sem_create("lock_staus",1);
    INTERSECTION->status = (int *)kmalloc (4*sizeof(int));
    INTERSECTION->waitlist_quad = (struct queue **)kmalloc (4*sizeof(struct queue *));
    INTERSECTION->waitlist_lock = (struct semaphore **)kmalloc(4*sizeof(struct semaphore *));
    
    INTERSECTION->waitlist_lock[NW] = sem_create("NW_waitlist ", 1);
    INTERSECTION->waitlist_lock[NE] = sem_create("NE_waitlist ", 1);
    INTERSECTION->waitlist_lock[SE] = sem_create("SE_waitlist ", 1);
    INTERSECTION->waitlist_lock[SW] = sem_create("SW_waitlist ", 1);

    /* 
    * Intially there is no car in the intersection.
    * So intiallize all quadrant status to -1
    */
    int i = 0;
    for (i = 0; i < 4; i++){
        INTERSECTION->status[i] = -1;
        INTERSECTION->waitlist_quad[i] = q_create(NCARS);
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
