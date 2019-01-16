/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	userlist_1;
qid16	userlist_2;
qid16	userlist_3;

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;

	if(prptr->user_process==0){

		//kprintf("%d pID\n",pid);
		insert(pid, readylist, prptr->prprio);
		//resched();

	}

	else{

		if(prptr->number_bursts>0){
			switch(prptr->queue_num){

				case 1:
						enqueue(pid, userlist_1);
						//insert(pid, userlist_1, prptr->run_time);
						break;
				case 2:
						enqueue(pid, userlist_2);
						//insert(pid, userlist_2, prptr->run_time);
						break;
				case 3:
						enqueue(pid, userlist_3);
						//insert(pid, userlist_3, prptr->run_time);
						break;
				default:
						break;
			}
		}

		//resched();

	}


	return OK;
}
