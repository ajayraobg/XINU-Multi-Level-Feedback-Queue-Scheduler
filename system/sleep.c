/* sleep.c - sleep sleepms */

#include <xinu.h>

#define	MAXSECONDS	2147483		/* Max seconds per 32-bit msec	*/

/*------------------------------------------------------------------------
 *  sleep  -  Delay the calling process n seconds
 *------------------------------------------------------------------------
 */
syscall	sleep(
	  int32	delay		/* Time to delay in seconds	*/
	)
{
	

	if ( (delay < 0) || (delay > MAXSECONDS) ) {
		return SYSERR;
	}
	sleepms(1000*delay);

	

	return OK;
}

/*------------------------------------------------------------------------
 *  sleepms  -  Delay the calling process n milliseconds
 *------------------------------------------------------------------------
 */
syscall	sleepms(
	  int32	delay			/* Time to delay in msec.	*/
	)
{
	

	intmask	mask;			/* Saved interrupt mask		*/

	if (delay < 0) {
				
		return SYSERR;
	}

	if (delay == 0) {
		yield();
				
		return OK;
	}

	/* Delay calling process */

	mask = disable();
	
	if (insertd(currpid, sleepq, delay) == SYSERR) {
		restore(mask);
		return SYSERR;
	}

	proctab[currpid].prstate = PR_SLEEP;

	uint32 curr_time1, curr_time2;
	curr_time1 = clktime*1000+ctr1000;
	curr_time2 = curr_time1;
/*	while(curr_time2<curr_time1+quantum*1000){
		curr_time2 = clktime*1000+ctr1000;
	}*/
	
	//resched();


		

	restore(mask);

	return OK;
}
