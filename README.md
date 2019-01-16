# XINU-Multi-Level-Feedback-Queue-Scheduler
This project implements the Multi-Level Feedback Queue (MLFQ) scheduling policy in Xinu.

The MLFQ scheduling policy operates according to the following rules:
• Rule 1: if Priority(A)>Priority (B), A runs
• Rule 2: if Priority(A)=Priority(B), A&B run in RR fashion
• Rule 3: initially a job is placed at the highest priority level
• Rule 4: once a job uses up its time allotment TAL at a given level, its priority is reduced
• Rule 5: after some time period S, move all jobs in the topmost queue

1. Implemented the MLFQ scheduling policy in Xinu. The implementation follows the following directions:
• System processes are scheduled with higher priority 
• There are 3 priority levels (i.e., 3 queues) for user processes.
• The time slice increases by a factor 2 when moving from higher to lower priority (i.e., TSL-1 = 2 TSL).
• The time slice at the highest priority level (TSHPL), the time allotment (TA) and the priority boost period S (all
  expressed in milliseconds) is configurable and defined in include/resched.h as follows:
  a. #define TIME_SLICE (TSHPL)
  b. #define TIME_ALLOTMENT (TA)
  c. #define PRIORITY_BOOST_PERIOD (S)
• System processes are scheduled using TSHPL (e.g., the same time slice as highest priority user
processes).
2. Implemented the function
void burst_execution(uint32 number_bursts, burst_duration, sleep_duration);
which simulates the execution of applications that alternate execution phases requiring the CPU (CPU bursts),
and execution phases not requiring it (CPU inactivity phases). Specifically:
- number_bursts = number of CPU bursts
- burst_duration = duration of each CPU burst (all CPU burst have the same duration)
- sleep_duration = duration of each CPU inactivity phase (all CPU inactivity phases have the same
duration)
Used this function to create use cases to validate the correct operation of the scheduler.
