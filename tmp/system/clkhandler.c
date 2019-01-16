/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/

	/* Decrement the ms counter, and see if a second has passed */

	
	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;

	}

	ctr1000 = 1000-count1000; //Added by Ajay

	
	/*if(boost_time == PRIORITY_BOOST_PERIOD){
		resched();
	}*/

	if(proctab[currpid].prstate== PR_CURR && proctab[currpid].user_process==1 && proctab[currpid].burst_duration>0 && proctab[currpid].number_bursts>0){

		--proctab[currpid].burst_duration;
		++boost_time;
		++proctab[currpid].alotted_time_used;
		if(proctab[currpid].burst_duration == 0)
			proctab[currpid].burst_completed = 1;
		/*if(proctab[currpid].alotted_time_used == TIME_ALLOTMENT)
			resched();*/

	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		//preempt = QUANTUM;
		//kprintf("\nCalled from clk\n");
		resched();
	}
}
