/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	pid32 old_pid,new_pid,curr;
	int flag = 0;

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];


	if(boost_time >= PRIORITY_BOOST_PERIOD){

		boost_time = 0;
		flag =1;


		if(ptold->user_process == 1){

			switch(ptold->queue_num){
				case 1:
						ptold->alotted_time_used = 0;
						enqueue(currpid,userlist_1);
						break;
				case 2:
						ptold->queue_num = 1;
						ptold->alotted_time_used = 0;
						enqueue(currpid,userlist_2);
						break;
				case 3:
						ptold->queue_num = 1;
						ptold->alotted_time_used = 0;
						enqueue(currpid,userlist_3);
						break;
			}

		}

		if(nonempty(userlist_1)){
			curr = firstid(userlist_1);
			while(curr!=queuetail(userlist_1)){
				proctab[curr].alotted_time_used = 0;
				curr = queuetab[curr].qnext;
			}

		}
		if(nonempty(userlist_2)){
			while(nonempty(userlist_2)){
				curr = dequeue(userlist_2);
				//kprintf("PID:%d\n",curr);
				proctab[curr].queue_num = 1;
				proctab[curr].alotted_time_used = 0;
				enqueue(curr,userlist_1);
			}
		}
		if(nonempty(userlist_3)){
			while(nonempty(userlist_3)){
				curr = dequeue(userlist_3);
				proctab[curr].queue_num = 1;
				proctab[curr].alotted_time_used = 0;
				enqueue(curr,userlist_1);
			}
		}
		if(nonempty(sleepq)){
			curr = firstid(sleepq);
			while(curr!=queuetail(sleepq)){
				if(proctab[curr].user_process==1){
					switch(proctab[curr].queue_num){
						case 1:
								proctab[curr].alotted_time_used = 0;
								break;
						case 2:
						case 3:
								proctab[curr].queue_num = 1;
								proctab[curr].alotted_time_used = 0;
								break;
					}
				}
				curr = queuetab[curr].qnext;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(ptold->user_process == 1){ //Check if reschedule was called by a user process

		if(ptold->prstate == PR_CURR){ //if the user process was running we need to put it into userlist if we are changing it from running to ready

			if(ptold->alotted_time_used >= TIME_ALLOTMENT){ //check if we arrived to resched because the user process used up its time alloted in the current level it is in and if yes then downgrade its priority

				ptold->prstate = PR_READY;

				if(ptold->number_bursts>0 && flag==0){

					switch(ptold->queue_num){

						case 1:
								ptold->queue_num = 2;
								ptold->alotted_time_used = 0;
								ptold->prstate = PR_READY;
								enqueue(currpid,userlist_2);
								break;
						case 2:
						case 3:
								ptold->queue_num = 3;
								ptold->alotted_time_used = 0;
								ptold->prstate = PR_READY;
								enqueue(currpid,userlist_3);
								break;

					}
				}

				if(!(strcmp(proctab[firstid(readylist)].prname,"prnull")==0)){ //priority has to be given to system process, so check if there are any process other than prnull in readylist

					//if there is a system process ready other than null insert the userproc in userlist and context switch
					old_pid = currpid;					
					currpid = dequeue(readylist);
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}

				else{ //there are no system process other than prnull in ready state

					if(nonempty(userlist_1) || nonempty(userlist_2) || nonempty(userlist_3)) { //check if there userproc in ready state

						old_pid = currpid;
						if(nonempty(userlist_1)){ //first priority given to user process in the first userlist queue
							currpid = dequeue(userlist_1);
							preempt = TIME_SLICE;
						}
						else if(nonempty(userlist_2)){ //second priority to user process in the second userlist queue
							currpid = dequeue(userlist_2);
							preempt = TIME_SLICE*2;
						}
						else{ //last priority to user process in the last userlist queue
							currpid = dequeue(userlist_3);
							preempt = TIME_SLICE*4;
						}
						new_pid = currpid;
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						//if(1){
						if(old_pid!=new_pid){
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif
							ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
						}

					}

					else{ //there are no user process or system process other than NULL process to run, so run null process

						old_pid = currpid;
						currpid = dequeue(readylist);
						new_pid = currpid;
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE;
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif	
						//kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

					}

				}

				
			}

			else{ //the user process was running and we did not arrived to resched because the user process used up its time alloted in the current level 

				if(ptold->number_bursts>0){
							ptold->prstate = PR_READY;
							if(flag==0){
								switch(ptold->queue_num){
									case 1:
											enqueue(currpid,userlist_1);
											break;
									case 2:
											enqueue(currpid,userlist_2);
											break;
									case 3:
											enqueue(currpid,userlist_3);
											break;
									default:
											break;
								}
							}
				}

				if(!(strcmp(proctab[firstid(readylist)].prname,"prnull")==0)){ //priority has to be given to system process, so check if there are any process other than prnull in readylist

						old_pid = currpid;
						
						currpid = dequeue(readylist);
						new_pid = currpid;
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE;
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif	
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}

				else if(nonempty(userlist_1) || nonempty(userlist_2) || nonempty(userlist_3)){

					old_pid = currpid;
					if(nonempty(userlist_1)){ //first priority given to user process in the first userlist queue
						/*if(ptold->number_bursts>0){
							ptold->prstate = PR_READY;
							enqueue(currpid,userlist_1);
						}*/
						currpid = dequeue(userlist_1);
						preempt = TIME_SLICE;
					}
					else if(nonempty(userlist_2)){ //second priority to user process in the second userlist queue
						/*if(ptold->number_bursts>0){
							ptold->prstate = PR_READY;
							enqueue(currpid,userlist_2);
						}*/
						currpid = dequeue(userlist_2);
						preempt = TIME_SLICE*2;
					}
					else{ //last priority to user process in the last userlist queue
						/*if(ptold->number_bursts>0){
							ptold->prstate = PR_READY;
							enqueue(currpid,userlist_3);
						}*/
						currpid = dequeue(userlist_3);
						preempt = TIME_SLICE*4;
					}
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
						if(old_pid!=new_pid){
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif	
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}
				}

				/*else if (ptold->number_bursts>0){ //there are no system or user process to run other than null and the current user process. So run the current user process.
					return;
				}*/

				else{ //run the NULL process

					old_pid = currpid;
					currpid = dequeue(readylist);
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;	
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif		
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
				}

			}
		}

		else{//if the userproc was not in running state and reschedule was called from a userproc

			if(!(strcmp(proctab[firstid(readylist)].prname,"prnull")==0)){ //priority has to be given to system process, so check if there are any process other than prnull in readylist

				old_pid = currpid;
				currpid = dequeue(readylist);
				new_pid = currpid;
				ptnew = &proctab[currpid];
				ptnew->prstate = PR_CURR;
				preempt = TIME_SLICE;	
				#ifdef CTXSW
					//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
					kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
				#endif	
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
			}

			else{ //there are no system process to run other than null, so check if there are other userproc to run

				if(nonempty(userlist_1) || nonempty(userlist_2) || nonempty(userlist_3)){ //if the userproc list is non empty, then pick the one with least execution time and context switch

					old_pid = currpid;
					if(nonempty(userlist_1)){
						currpid = dequeue(userlist_1);
						preempt = TIME_SLICE;
					}
					else if(nonempty(userlist_2)){
						currpid  = dequeue(userlist_2);
						preempt = TIME_SLICE*2;
					}
					else {
						currpid = dequeue(userlist_3);
						preempt = TIME_SLICE*4;
					}
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif	
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}

				else{ //there are no userproc to run, so run the null process

					old_pid = currpid;
					currpid = dequeue(readylist);
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;	
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif	
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}

			}

		}

	}

	else{ //reschdule was called by a system process

		if(!(strcmp(ptold->prname,"prnull")==0)){ //current system process is not null process

			if (ptold->prstate == PR_CURR){ //the state of the system process is running

				if (ptold->prprio > firstkey(readylist)){ //the current process is the highest priority ready process so no context switching
					preempt = TIME_SLICE;
					//kprintf("here 1\n");
					return;
				}

				else{ //the current process is not the highest priority ready process so need to perform context switching to highest priority ready process

					//kprintf("here 2\n");
					old_pid = currpid;
					ptold->prstate = PR_READY;
					insert(currpid, readylist, ptold->prprio);
					currpid = dequeue(readylist);
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;	
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif	
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}

			}

			else{ //the state of the system process is not running so can perform context switching without checking priority

				if(!(strcmp(proctab[firstid(readylist)].prname,"prnull")==0)){//there is a system process other than null in the ready list

					//kprintf("here 3\n");
					old_pid = currpid;
					currpid = dequeue(readylist);
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;	
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif	
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}

				else{ //there is no system process other than null that is in ready state and the current process is not running so we can run user process if any is ready

					if(nonempty(userlist_1) || nonempty(userlist_2) || nonempty(userlist_3)){ //there is a user process to run

						//kprintf("here 4\n");
						old_pid = currpid;
						if(nonempty(userlist_1)){ //first priority given to user process in the first userlist queue
							
							currpid = dequeue(userlist_1);
							preempt = TIME_SLICE;
						}
						else if(nonempty(userlist_2)){ //second priority to user process in the second userlist queue
							currpid = dequeue(userlist_2);
							preempt = TIME_SLICE*2;
						}
						else{ //last priority to user process in the last userlist queue
							currpid = dequeue(userlist_3);
							preempt = TIME_SLICE*4;
						}
						new_pid = currpid;
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif	
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

					}

					else{ //have to run the null process

						//kprintf("here 5\n");
						old_pid = currpid;
						currpid = dequeue(readylist);
						new_pid = currpid;
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						preempt = TIME_SLICE;
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif		
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
					}

				}

			}
		}

		else{ //current system process is null process so we can run user process if there are no system process

			if (ptold->prstate == PR_CURR){ //null process was in running state

				if(isempty(readylist)){ //there are no system process in ready state

					if(nonempty(userlist_1) || nonempty(userlist_2) || nonempty(userlist_3)){ //there are userproc in ready state so context switch to userproc adding null process to readylist for system process

						//kprintf("here 6\n");
						old_pid = currpid;
						ptold->prstate = PR_READY;
						insert(currpid, readylist, ptold->prprio);
						if(nonempty(userlist_1)){ //first priority given to user process in the first userlist queue
							currpid = dequeue(userlist_1);
							preempt = TIME_SLICE;
						}
						else if(nonempty(userlist_2)){ //second priority to user process in the second userlist queue
							currpid = dequeue(userlist_2);
							preempt = TIME_SLICE*2;
						}
						else{ //last priority to user process in the last userlist queue
							currpid = dequeue(userlist_3);
							preempt = TIME_SLICE*4;
						}
						new_pid = currpid;
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif	
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

					}

					else{
						preempt = TIME_SLICE;
						//kprintf("here\n"); //there is no other user or system process ready to run
						return;
					}
				}

				else{ //there are system process ready to run so give them priority

					//kprintf("here 7\n");
					old_pid = currpid;
					ptold->prstate = PR_READY;
					insert(currpid, readylist, ptold->prprio);
					currpid = dequeue(readylist);
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;	
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif		
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}
			}

			else{ //null process was not in running state

				if(isempty(readylist)){ //there are no system process in ready state

					if(nonempty(userlist_1) || nonempty(userlist_2) || nonempty(userlist_3)){ //there are userproc in ready state so context switch to userproc

						//kprintf("here 8\n");
						old_pid = currpid;
						if(nonempty(userlist_1)){ //first priority given to user process in the first userlist queue
							currpid = dequeue(userlist_1);
							preempt = TIME_SLICE;
						}
						else if(nonempty(userlist_2)){ //second priority to user process in the second userlist queue
							currpid = dequeue(userlist_2);
							preempt = TIME_SLICE*2;
						}
						else{ //last priority to user process in the last userlist queue
							currpid = dequeue(userlist_3);
							preempt = TIME_SLICE*4;
						}
						new_pid = currpid;
						ptnew = &proctab[currpid];
						ptnew->prstate = PR_CURR;
						#ifdef CTXSW
							//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
							kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
						#endif	
						ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

					}

					else {
						preempt = TIME_SLICE;//there is no other user or system process ready to run
						return;
					}
				}

				else{ //there are system process ready to run so give them priority

					//kprintf("here 9\n");
					old_pid = currpid;
					currpid = dequeue(readylist);
					new_pid = currpid;
					ptnew = &proctab[currpid];
					ptnew->prstate = PR_CURR;
					preempt = TIME_SLICE;
					#ifdef CTXSW
						//kprintf("ctxsw::%d-%d (%d)\n",old_pid,new_pid,(clktime*1000+ctr1000));
						kprintf("ctxsw::%d-%d\n",old_pid,new_pid);
					#endif	
					ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

				}


			}
		}
		
	}



















	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	/*if (ptold->prstate == PR_CURR) {  
		if (ptold->prprio > firstkey(readylist)) {
				
			return;
		}

		

		
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);*/

	
	

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
